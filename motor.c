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



void motor_init(void)
{
    //Initialize timer 2
    //Use timer2 for CCP1 module, timer 4 for CCP2 module
    TCLKCONbits.T3CCP2 = 0b0;
    TCLKCONbits.T3CCP1 = 0b1;
    
    //Post scaler = 16
    T2CONbits.T2OUTPS = 0b1111;
    
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
    //Set output pins
    MOTOR_ENABLE_PIN = 0; //Enable drive
    if(direction==MOTOR_DIRECTION_CCW)
        MOTOR_DIRECTION_PIN = 0;
    else
        MOTOR_DIRECTION_PIN = 1;
    //Initialize variables
    motor_direction = direction;
    motor_milliseconds = MILLISECONDS_START;
    motor_current_stepcount = STEPCOUNT_START;
    motor_final_stepcount = distance << FULL_STEP_SHIFT;
    motor_half_stepcount = (motor_final_stepcount>>1) - 1;
    //Calculate speed to start with and apply it
    motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
    T2CONbits.T2CKPS = 0b10;
    PR2 = motor_next_change_pwmperiod>>4;
    //Set duty cycle to 50%
    CCPR1L = PR2>>1;
    //Prepare next move
    ++motor_milliseconds;
    motor_next_change_stepcount = motor_milliseconds;
    motor_next_change_stepcount *= motor_milliseconds;
    motor_next_change_stepcount *= POSITION_MULTIPLIER;
    motor_next_change_stepcount >>= POSITION_SHIFT;
    motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
    //Set output pins
    MOTOR_ENABLE_PIN = 0; //Enable drive
    if(direction==MOTOR_DIRECTION_CCW)
        MOTOR_DIRECTION_PIN = 0;
    else
        MOTOR_DIRECTION_PIN = 1;
    //Configure interrupts
    PIR1bits.TMR2IF = 0;
    PIE1bits.TMR2IE = 1;
    //Clear and start timer
    TMR2 = 0;
    T2CONbits.TMR2ON = 1;
}

void motor_isr(void)
{
    //Keep track of position
    ++motor_current_stepcount;
    if((motor_current_stepcount&FULL_STEP_MASK)==0)
    {
        os.current_position += motor_direction;
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
        //Set new period
        if(motor_next_change_pwmperiod>4095)
        {
            //Prescaler = 16
            T2CONbits.T2CKPS = 0b10;
            PR2 = motor_next_change_pwmperiod>>4;
        }
        else if(motor_next_change_pwmperiod>1023)
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
        PR2 = motor_next_change_pwmperiod>>4;
        //Set duty cycle to 50%
        CCPR1L = PR2>>1;
        //Calculate next speed (and when to change it)
        ++motor_milliseconds;
        if(motor_milliseconds>ACCELERATION_PERIOD)
        {
            //Don't change speed any more
            motor_next_change_stepcount = motor_final_stepcount;
        }
        else
        {
            motor_next_change_stepcount = motor_milliseconds;
            motor_next_change_stepcount *= motor_milliseconds;
            motor_next_change_stepcount *= POSITION_MULTIPLIER;
            motor_next_change_stepcount >>= POSITION_SHIFT;
            motor_next_change_pwmperiod = ((uint32_t)TIME_BASE) / motor_milliseconds;
        }
    }
    
    //Clear interrupt flag
    PIR1bits.TMR2IF = 0;
}

uint16_t motor_get_new_stepsize(uint16_t old_stepsize)
{
    switch(old_stepsize)
    {
        case 1:
            return 10;
        case 10:
            return 100;
        case 100:
            return 1000;
        case 1000:
            return 1;
        default:
            return 100;
    }
}
