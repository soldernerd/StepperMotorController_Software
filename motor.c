#include <stdint.h>
#include <xc.h>
#include <pic18f26j50.h>
#include "motor.h"
#include "os.h"

uint16_t motor_milliseconds;
uint32_t motor_current_stepcount;
uint32_t motor_final_stepcount;
uint32_t motor_half_stepcount;
motorDirection_t motor_direction;

uint32_t motor_next_change_stepcount;
uint16_t motor_next_change_pwmperiod;

uint32_t tmp;

void motor_init(void)
{
    //Initialize timer 2
    //Use timer2 for CCP1 module, timer 4 for CCP2 module
    TCLKCONbits.T3CCP2 = 0b0;
    TCLKCONbits.T3CCP1 = 0b1;
    
    //Post scaler = 16
    //T2CONbits.T2OUTPS = 0b1111;
    //Post scaler = 1
    T2CONbits.T2OUTPS = 0b0000;
    
    //Prescaler = 16
    T2CONbits.T2CKPS = 0b10;
    //T2CONbits.T2CKPS0 = 0;
    PR2 = 100;
    
    //Disable timer 2
    T2CONbits.TMR2ON = 0;
    
    //Single output mode
    CCP1CONbits.P1M = 0b00;
    //PWM mode both outputs active high
    CCP1CONbits.CCP1M = 0b1100;
    //Duty cycle LSBs
    CCP1CONbits.DC1B = 0b00;
    //Duty cycle high MSBs
    CCPR1L = PR2>>1;
}

void motor_run(motorDirection_t direction, uint16_t distance)
{
    //Save direction
    motor_direction = direction;
    
    //Initialize variables, calculate distances
    motor_milliseconds = MILLISECONDS_START;
    motor_current_stepcount = 0;
    motor_final_stepcount = distance;
    motor_final_stepcount <<= FULL_STEP_SHIFT;
    motor_half_stepcount = (motor_final_stepcount>>1) - 1;
    
    //Set output pins
    MOTOR_ENABLE_PIN = 0; //Enable drive
    if(direction==MOTOR_DIRECTION_CCW)
        MOTOR_DIRECTION_PIN = 0;
    else
        MOTOR_DIRECTION_PIN = 1;

    //Set prescaler to maximum value of 16
    T2CONbits.T2CKPS = 0b10;
    //Set PWM period to maximum value of 255
    PR2 = 255;
    //Set duty cycle to 50%
    CCPR1L = PR2>>1;

    //Prepare next move
    motor_next_change_stepcount = motor_milliseconds;
    motor_next_change_stepcount *= motor_milliseconds;
    motor_next_change_stepcount *= POSITION_MULTIPLIER;
    motor_next_change_stepcount >>= POSITION_SHIFT;
    motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;

    //Configure interrupts
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    //Clear and start timer
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
    
    //Indicate that the motor is running
    os.busy = 1;
}

void motor_isr(void)
{
    //Keep track of position
    ++motor_current_stepcount;
    if((motor_current_stepcount&FULL_STEP_MASK)==0)
    {
        os.current_position += motor_direction;
        if(os.current_position==36000)
            os.current_position = 0;
        if(os.current_position==0xFFFF)
            os.current_position = 35999;
        //Check if we are done
        if(motor_current_stepcount==motor_final_stepcount)
        {
           T2CONbits.TMR2ON = 0;
           MOTOR_ENABLE_PIN = 1; //disable
           PIR1bits.TMR2IF = 0;
           PIE1bits.TMR2IE = 0;
           return;
        }
    }
    
    //Check if we need to change speed
    if(motor_current_stepcount==motor_next_change_stepcount)
    {
        //Change speed by setting prescaler and PWM Period
        if(motor_next_change_pwmperiod>1023)
        {
            //Prescaler = 16
            T2CONbits.T2CKPS = 0b10;
            PR2 = motor_next_change_pwmperiod>>4;
        }
        else if(motor_next_change_pwmperiod>255)
        {
            //Prescaler = 4
            T2CONbits.T2CKPS = 0b01;
            PR2 = motor_next_change_pwmperiod>>2;
        }
        else
        {
            //Prescaler = 1
            T2CONbits.T2CKPS = 0b00;
            PR2 = motor_next_change_pwmperiod;
        } 
        //Set duty cycle to 50%
        CCPR1L = PR2>>1;
        
        //Calculate the next speed and when to apply it
        
        //Check if we should accelerate or de-accelerate
        if(motor_current_stepcount<motor_half_stepcount)
        {
            //We are still in the first half of the trajectory, maybe accelerate
            if(motor_milliseconds<ACCELERATION_PERIOD)
            {
                //Increment time
                ++motor_milliseconds;
                //Calculate next PWP period
                motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
                //Calculate next position where speed should be changed
                motor_next_change_stepcount = motor_milliseconds;
                motor_next_change_stepcount *= motor_milliseconds;
                motor_next_change_stepcount *= POSITION_MULTIPLIER;
                motor_next_change_stepcount >>= POSITION_SHIFT;
            }
            else
            {
                //Let's re-calculate the next speed & position once we've completed half of our trajectory
                motor_next_change_stepcount = motor_half_stepcount;
            }
        }
        else
        {
            //We are already in the second half of the trajectory
            //De-accelerate for sure, the only question is when
            
            //Calculate next position where speed should be changed
            //Yes, calculate that before decrementing time
            tmp = motor_milliseconds;
            tmp *= motor_milliseconds;
            tmp *= POSITION_MULTIPLIER;
            tmp >>= POSITION_SHIFT;
            //Subtract this stepcount from the final stepcount (we're counting backwards now, remember?)
            motor_next_change_stepcount = motor_final_stepcount - tmp;
            
            //Decrement time, we're counting backwards now
            --motor_milliseconds;
            
            //Calculate next PWP period
            motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
            //Make sure PWM period is not longer than allowed by the hardware
            if(motor_next_change_pwmperiod>MAXIMUM_PWM_PERIOD)
            {
                motor_next_change_pwmperiod>MAXIMUM_PWM_PERIOD;
            }
        }
        
//        ++motor_milliseconds;
//        if(motor_milliseconds>ACCELERATION_PERIOD)
//        {
//            //Don't change speed any more
//            motor_next_change_stepcount = motor_final_stepcount;
//        }
//        else
//        {
//            motor_next_change_stepcount = motor_milliseconds;
//            motor_next_change_stepcount *= motor_milliseconds;
//            motor_next_change_stepcount *= POSITION_MULTIPLIER;
//            motor_next_change_stepcount >>= POSITION_SHIFT;
//            motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
//        }
    }
    
    //Clear interrupt flag
    PIR1bits.TMR2IF = 0;
}

void motor_start(motorDirection_t direction)
{
    //Save direction
    motor_direction = direction;
    
    //uint32_t motor_half_stepcount;
    
    //Set output pins
    MOTOR_ENABLE_PIN = 0; //Enable drive
    if(direction==MOTOR_DIRECTION_CCW)
        MOTOR_DIRECTION_PIN = 0;
    else
        MOTOR_DIRECTION_PIN = 1;

    //Set prescaler to maximum value of 16
    T2CONbits.T2CKPS = 0b10;
    //Set PWM period to maximum value of 255
    PR2 = 255;
    //Set duty cycle to 50%
    CCPR1L = PR2>>1;

    //Prepare next move
    motor_milliseconds = MILLISECONDS_START;
    motor_next_change_stepcount = motor_milliseconds;
    motor_next_change_stepcount *= motor_milliseconds;
    motor_next_change_stepcount *= POSITION_MULTIPLIER;
    motor_next_change_stepcount >>= POSITION_SHIFT;
    motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;

    //Configure interrupts
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    //Clear and start timer
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
    os.busy = 1;
    os.manual_speed = 0;
}

void motor_stop(void)
{
    T2CONbits.TMR2ON = 0;
    MOTOR_ENABLE_PIN = 1; //disable
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 0;
    os.busy = 0;
    os.manual_speed = 0;
}

void motor_change_speed(uint8_t new_speed)
{
    motor_milliseconds = MILLISECONDS_START+new_speed;
    motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
    
    //Set new period
    if(motor_next_change_pwmperiod>1023)
    {
        //Prescaler = 16
        T2CONbits.T2CKPS = 0b10;
        PR2 = motor_next_change_pwmperiod>>4;
    }
    else if(motor_next_change_pwmperiod>255)
    {
        //Prescaler = 4
        T2CONbits.T2CKPS = 0b01;
        PR2 = motor_next_change_pwmperiod>>2;
    }
    else
    {
        //Prescaler = 1
        T2CONbits.T2CKPS = 0b00;
        PR2 = motor_next_change_pwmperiod;
    } 
    
    //Set duty cycle to 50%
    CCPR1L = PR2>>1;
}