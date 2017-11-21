#include <stdint.h>
#include <xc.h>
#include <pic18f26j50.h>
#include "motor.h"
#include "motor_config.h"
#include "os.h"

motorMode_t motor_mode;
motorDirection_t motor_direction;
uint16_t motor_maximum_speed;

volatile uint16_t motor_current_speed;
volatile uint32_t motor_current_stepcount;
volatile uint32_t motor_final_stepcount;
volatile uint32_t motor_next_speed_check;

motorMode_t motor_get_mode(void)
{
    return motor_mode;
}

uint16_t motor_get_maximum_speed(void)
{
    return motor_speed_lookup[motor_maximum_speed];
}

uint16_t motor_get_current_speed(void)
{
    return motor_speed_lookup[motor_current_speed];
}

uint16_t motor_speed_from_index(uint8_t speed_index)
{
    return motor_speed_lookup[speed_index];
}

void motor_init(void)
{
    //Initialize timer 2
    //Use timer2 for CCP1 module, timer 4 for CCP2 module
    TCLKCONbits.T3CCP2 = 0b0;
    TCLKCONbits.T3CCP1 = 0b1;
    
    //Single output mode
    CCP1CONbits.P1M = 0b00;
    
    //Duty cycle LSBs
    CCP1CONbits.DC1B = 0b00;
    
    //Enable drive
    MOTOR_ENABLE_PIN = 0;
}

void motor_run(motorDirection_t direction, uint16_t distance, uint8_t speed)
{
    //Save direction
    motor_direction = direction;
    
    //Maximum speed
    if(speed==0)
    {
        motor_maximum_speed = MAXIMUM_SPEED;
    }
    else
    {
        motor_maximum_speed = speed;
    }
    
    //Initialize variables, calculate distances
    motor_current_speed = 0;
    motor_current_stepcount = 0;
    motor_final_stepcount = distance;
    motor_final_stepcount <<= FULL_STEP_SHIFT;
    motor_next_speed_check = motor_steps_lookup[1];
    
    //Disable PWM module
    CCP1CONbits.CCP1M = 0b0000;
    
    //Set output pins
    MOTOR_ENABLE_PIN = 0; //Enable drive
    if(direction==MOTOR_DIRECTION_CCW)
        MOTOR_DIRECTION_PIN = 0;
    else
        MOTOR_DIRECTION_PIN = 1;
    
    //Manually control step
    MOTOR_STEP_PIN = 0;
    PPSUnLock();
    MOTOR_STEP_PPS = 0;
    PPSLock();
    
    //Set motor mode
    motor_mode = MOTOR_MODE_MANUAL;
    
    //Set up timer 2 and PWM module
    //Prescaler
    T2CONbits.T2CKPS = motor_prescaler_lookup[motor_current_speed];
    //Period
    PR2 = motor_period_lookup[motor_current_speed];
    //Postscaler
    T2CONbits.T2OUTPS = motor_postscaler_lookup[motor_current_speed];
    //Duty cycle = 50%
    CCPR1L = PR2>>1;
    
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
    uint8_t step_count;
    uint32_t steps_remaining;
    uint16_t steps_until_standstill;
    uint16_t steps_until_standstill_if_accelerate;
    
    //Clear interrupt flag
    PIR1bits.TMR2IF = 0;
    
    if(motor_mode==MOTOR_MODE_MANUAL)
    {
        //Need to toggle step pin manually
        if(MOTOR_STEP_PIN)
        {
            //Set pin low
            MOTOR_STEP_PIN = 0;
            
            //Check if we are done altogether
            if((motor_current_stepcount+step_count)>=motor_final_stepcount)
            {
                //We are done. Stop everything
                //Disable timer, disable motor drive, clear and disable interrupts
                T2CONbits.TMR2ON = 0;
                //MOTOR_ENABLE_PIN = 1; //disable
                PIR1bits.TMR2IF = 0;
                PIE1bits.TMR2IE = 0;
                os.busy = 0;
                //Make sure we do not go through the change speed code
                motor_next_speed_check = motor_final_stepcount + 1;
            }
        }
        else
        {
            //Set pin high
            MOTOR_STEP_PIN = 1;
            //Increment on rising edge only
            step_count = 1;
        }
    }
    else
    {
        //Just increment by 1 step
        step_count = 1;
    }
    
    //Check if we need to (maybe) change speed.
    if((motor_current_stepcount+step_count)>=motor_next_speed_check)
    {  
        //Calculate some basic values
        if(motor_final_stepcount>motor_current_stepcount)
            steps_remaining = motor_final_stepcount - motor_current_stepcount - step_count;
        else
            steps_remaining = 0;
        steps_until_standstill = motor_steps_lookup[motor_current_speed];
        steps_until_standstill_if_accelerate = motor_steps_lookup[motor_current_speed+2];
                
        if((motor_current_speed>motor_maximum_speed) || (steps_until_standstill>=steps_remaining))
        {
            //Need to de-accelerate
            --motor_current_speed;
            
            //Check if we need to change drive mode
            if((motor_mode==MOTOR_MODE_PWM) && (motor_postscaler_lookup[motor_current_speed]>0))
            {
                //Need to change from PWM mode to manual mode
                motor_mode = MOTOR_MODE_MANUAL;
            
                //Control steps manually
                MOTOR_STEP_PIN = 1;
                PPSUnLock();
                MOTOR_STEP_PPS = 0;
                PPSLock();
                
                //clear timer
                TMR2 = 0;
                
                //Turn off PWM module
                CCP1CONbits.CCP1M = 0b0000;
            }
        
            //Update parameters
            //Prescaler
            T2CONbits.T2CKPS = motor_prescaler_lookup[motor_current_speed];
            //Period
            PR2 = motor_period_lookup[motor_current_speed];
            //Postscaler
            T2CONbits.T2OUTPS = motor_postscaler_lookup[motor_current_speed];
            //Duty cycle = 50%
            CCPR1L = PR2>>1;
            
            //Set when to de-accelerate next time
            motor_next_speed_check = motor_final_stepcount - motor_steps_lookup[motor_current_speed-1];
        }
        else if((motor_current_speed==motor_maximum_speed) || (steps_until_standstill_if_accelerate>=steps_remaining))
        {
            //Maintain current speed
            
            //Calculate when to revise speed next time
            motor_next_speed_check = motor_current_stepcount + motor_steps_lookup[motor_current_speed+1] - motor_steps_lookup[motor_current_speed];
            //Make sure we don't miss the time to stop
            if((motor_final_stepcount-steps_until_standstill) < motor_next_speed_check)
            {
               motor_next_speed_check = motor_final_stepcount - steps_until_standstill;
            }
        }
        else
        {
            //can accelerate further
            ++motor_current_speed;
            
            if((motor_mode==MOTOR_MODE_MANUAL) && (motor_postscaler_lookup[motor_current_speed]==0))
            {
                //Need to change from manual mode to PWM mode
                motor_mode = MOTOR_MODE_PWM;

                //Enable PWM module
                CCP1CONbits.CCP1M = 0b1100;
                
                //Control step pin via PWM module
                PPSUnLock();
                MOTOR_STEP_PPS = PPS_FUNCTION_CCP1_OUTPUT;
                PPSLock();
            }
            
            //Update parameters
            //Prescaler
            T2CONbits.T2CKPS = motor_prescaler_lookup[motor_current_speed];
            //Period
            PR2 = motor_period_lookup[motor_current_speed];
            //Postscaler
            T2CONbits.T2OUTPS = motor_postscaler_lookup[motor_current_speed];
            //Duty cycle = 50%
            CCPR1L = PR2>>1;
            
            //Calculate when to revise speed next time
            motor_next_speed_check = motor_current_stepcount + motor_steps_lookup[motor_current_speed+1] - motor_steps_lookup[motor_current_speed];
        }
    }
    

    //Check if another interrupt has occured in the mean time
    if(PIR1bits.TMR2IF)
    {
        //Clear interrupt flag
        PIR1bits.TMR2IF = 0;
        //Increment step count accordingly
        ++step_count;
    }
    
    //Keep track of current position
    while(step_count)
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
        --step_count;
    }
}

void motor_stop(void)
{
    motor_final_stepcount = motor_current_stepcount + motor_steps_lookup[motor_current_speed];
}

void motor_change_speed(uint8_t new_speed)
{
    motor_maximum_speed = (uint16_t) new_speed;
}