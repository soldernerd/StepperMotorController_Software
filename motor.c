#include <stdint.h>
#include <xc.h>
#include <pic18f26j50.h>
#include "motor.h"
#include "motor_config.h"
#include "os.h"

motorMode_t motor_mode;

//uint16_t motor_milliseconds;
uint16_t motor_current_speed;
uint32_t motor_current_stepcount;
uint32_t motor_final_stepcount;
uint32_t motor_half_stepcount;
motorDirection_t motor_direction;

uint32_t motor_next_change_stepcount;
//uint16_t motor_next_change_pwmperiod;
uint8_t motor_next_change_prescaler;
uint8_t motor_next_change_period;

uint32_t tmp;

void startup(void)
{
    MOTOR_ENABLE_PIN = 0; //Enable drive
    
    //Step low
    MOTOR_STEP_PIN = 0;
    
    //Manually control step
    PPSUnLock();
    MOTOR_STEP_PPS = 0;
    PPSLock();
    
    //Prescaler = 16
    T2CONbits.T2CKPS = 0b10;
    
    //Post-Scaler = 16
    T2CONbits.T2OUTPS = 0b1111;
    
    motor_current_stepcount = 0;
    PR2 =  motor_start_lookup[motor_current_stepcount];
    
    //Clear interrupt flag
    PIR1bits.TMR2IF = 0;
    //Disable interrupts
    PIE1bits.TMR2IE = 0;
    
    //Clear and start timer
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
    
    //while(motor_current_stepcount<motor_step_lookup[0]) 
    while(motor_current_stepcount<54) 
    {
        //++motor_current_stepcount;
        if(MOTOR_STEP_PIN)
        //if(steps&0b00000001)
        {
            MOTOR_STEP_PIN = 0;
            ++motor_current_stepcount;
            PR2 =  motor_start_lookup[motor_current_stepcount];
        }
        else
        {
            MOTOR_STEP_PIN = 1;
        }
        while(!PIR1bits.TMR2IF);
        PIR1bits.TMR2IF = 0;
    }
    
    MOTOR_STEP_PIN = 0;
    
    //Disable drive
    MOTOR_ENABLE_PIN = 1;
    
    //Post scaler = 1
    T2CONbits.T2OUTPS = 0b0000;
    
    //Stop timer
    T2CONbits.TMR2ON = 0;
    
    //Control step pin via PWM module
    PPSUnLock();
    MOTOR_STEP_PPS = PPS_FUNCTION_CCP1_OUTPUT;
    PPSLock();
}

void motor_init(void)
{
    uint16_t cntr;
    uint16_t tmp;
    
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
    motor_current_speed = 0;
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
    motor_next_change_stepcount = motor_step_lookup[motor_current_speed];
    motor_next_change_prescaler = motor_prescaler_lookup[motor_current_speed];
    motor_next_change_period = motor_period_lookup[motor_current_speed];

    //Configure interrupts
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    
    //Set motor mode
    motor_mode = MOTOR_MODE_RUN;
    
    //Clear and start timer
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
    
    //Indicate that the motor is running
    os.busy = 1;
}

void motor_isr(void)
{
    uint8_t interrupt_count;
    
    switch(motor_mode)
    {
        case MOTOR_MODE_RUN:
    
            //Clear interrupt flag
            PIR1bits.TMR2IF = 0;
            interrupt_count = 1;

            //Check if we are done
            if((motor_current_stepcount+1)==motor_final_stepcount)
            {
                //We are done. 
                //Disable timer, disable motor drive, clear and disable interrupts
                T2CONbits.TMR2ON = 0;
                MOTOR_ENABLE_PIN = 1; //disable
                PIR1bits.TMR2IF = 0;
                PIE1bits.TMR2IE = 0;

                //Keep track of position
                ++motor_current_stepcount;
                if((motor_current_stepcount&FULL_STEP_MASK)==0)
                {
                    os.current_position += motor_direction;
                    if(os.current_position==36000)
                        os.current_position = 0;
                    if(os.current_position==0xFFFF)
                        os.current_position = 35999;
                }

                return;
            }

            //Check if we need to change speed
            if(motor_current_stepcount==motor_next_change_stepcount)
            {
                //Set new prescaler and period
                T2CONbits.T2CKPS = motor_next_change_prescaler;
                PR2 = motor_next_change_period;
                //Set duty cycle to 50%
                CCPR1L = PR2>>1;

                //Check if next interrupt has already occured
                if(PIR1bits.TMR2IF)
                {
                    //Clear interrupt flag
                    PIR1bits.TMR2IF = 0;
                    //Increment interrupt counter
                    ++interrupt_count;
                    //Keep track of position
                }

                //Check whether to accelerate or de- accelerate
                if((motor_current_speed==MAXIMUM_SPEED) || (motor_current_stepcount>motor_half_stepcount) || (motor_step_lookup[motor_current_speed+1]>motor_half_stepcount))
                {
                    //De-accelerate
                    motor_next_change_stepcount = motor_final_stepcount - motor_step_lookup[motor_current_speed];
                    --motor_current_speed;
                    motor_next_change_prescaler = motor_prescaler_lookup[motor_current_speed];
                    motor_next_change_period = motor_period_lookup[motor_current_speed];
                }
                else
                {
                    //Accelerate
                    ++motor_current_speed;
                    motor_next_change_stepcount = motor_step_lookup[motor_current_speed];
                    motor_next_change_prescaler = motor_prescaler_lookup[motor_current_speed];
                    motor_next_change_period = motor_period_lookup[motor_current_speed];
                }
            }

            //Check if next interrupt has already occured
            if(PIR1bits.TMR2IF)
            {
                //Clear interrupt flag
                PIR1bits.TMR2IF = 0;
                //Increment interrupt counter
                ++interrupt_count;
            }

            //Keep track of current position
            while(interrupt_count)
            {
                ++motor_current_stepcount;
                if((motor_current_stepcount&FULL_STEP_MASK)==0)
                {
                    os.current_position += motor_direction;
                    if(os.current_position==36000)
                        os.current_position = 0;
                    if(os.current_position==0xFFFF)
                        os.current_position = 35999;
                }
                //Decrement count
                --interrupt_count;
            }
            
            break; //MOTOR_MODE_NORMAL
        
        case MOTOR_MODE_START:
            
            if(MOTOR_STEP_PIN)
            {
                MOTOR_STEP_PIN = 0;
                ++motor_current_stepcount;

                //Check if start phase is over
                if(motor_current_stepcount<54)
                {
                    //Start phase not over yet, load new value
                    PR2 =  motor_start_lookup[motor_current_stepcount];
                }
                else
                {
                    //Start phase is over, switch to proper pwm mode

                    //Control step pin via PWM module
                    PPSUnLock();
                    MOTOR_STEP_PPS = PPS_FUNCTION_CCP1_OUTPUT;
                    PPSLock();
                    
                    //Post scaler = 1
                    T2CONbits.T2OUTPS = 0b0000;
                    //Set Prescaler
                    T2CONbits.T2CKPS =  motor_prescaler_lookup[0];
                    //Set PWM period
                    PR2 = motor_period_lookup[0];
                    //Set duty cycle to 50%
                    CCPR1L = PR2>>1;
                    
                    //Calculate next values
                    motor_current_speed = 1;
                    motor_next_change_stepcount = motor_step_lookup[motor_current_speed];
                    motor_next_change_prescaler = motor_prescaler_lookup[motor_current_speed];
                    motor_next_change_period = motor_period_lookup[motor_current_speed];
                }
            }
            else
            {
                //Just set step pin high
                MOTOR_STEP_PIN = 1;
            }

            //Clear interrupt flag
            PIR1bits.TMR2IF = 0;

            break;
        
        case MOTOR_MODE_STOP:
            break;
    } //switch
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
    //motor_milliseconds = MILLISECONDS_START;
    motor_current_speed = 0;
    motor_next_change_stepcount = motor_step_lookup[motor_current_speed];
    motor_next_change_prescaler = motor_prescaler_lookup[motor_current_speed];
    motor_next_change_period = motor_period_lookup[motor_current_speed];
    //motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;

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
    motor_current_speed = new_speed;
    //Set new prescaler and period
    T2CONbits.T2CKPS = motor_prescaler_lookup[new_speed];
    PR2 = motor_period_lookup[new_speed];
    //Set duty cycle to 50%
    CCPR1L = PR2>>1;
}