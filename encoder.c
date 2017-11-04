#include <stdint.h>
#include <xc.h>
#include "encoder.h"
#include "os.h"
#include "motor.h"

#define COUNT_MIN -128
#define COUNT_MAX 127

uint8_t enc1;
uint8_t enc2;

void encoder_init(void)
{
   enc1 = ENCODER1_PORT & ENCODER1_MASK; 
   enc2 = ENCODER2_PORT & ENCODER2_MASK;
   os.encoder1Count = 0;
   os.encoder2Count = 0;
   os.button1 = 0;
   os.button2 = 0;
   os.displayState = DISPLAY_STATE_MAIN_SETUP;
   //os.displayState = DISPLAY_STATE_ENCODER_TEST;
}

void encoder_run(void)
{
    if(enc1 != (ENCODER1_PORT&ENCODER1_MASK))
    {
        //B rising while A high -> +
        if(ENCODER1_B_PIN && ENCODER1_A_PIN && (!(enc1&ENCODER1_B_MASK)))
        {
            if(os.encoder1Count<COUNT_MAX)
                ++os.encoder1Count;
        }
        //A rising while B high -> -
        if(ENCODER1_A_PIN && ENCODER1_B_PIN && (!(enc1&ENCODER1_A_MASK)))
        {
            if(os.encoder1Count>COUNT_MIN)
                --os.encoder1Count;
        }
        //Pushbutton pressed
        if(ENCODER1_PB_PIN && (!(enc1&ENCODER1_PB_MASK)))
        {
            os.button1 = 1;
        }
        //Pushbutton released
        if((!ENCODER1_PB_PIN) && (enc1&ENCODER1_PB_MASK))
        {
            os.button1 = -1;
        }
        //Save current state
        enc1 = ENCODER1_PORT & ENCODER1_MASK; 
    }
    
    if(enc2 != (ENCODER2_PORT&ENCODER2_MASK))
    {
        //B rising while A high -> +
        if(ENCODER2_B_PIN && ENCODER2_A_PIN && (!(enc2&ENCODER2_B_MASK)))
        {
            if(os.encoder2Count<COUNT_MAX)
                ++os.encoder2Count;
        }
        //A rising while B high -> -
        if(ENCODER2_A_PIN && ENCODER2_B_PIN && (!(enc2&ENCODER2_A_MASK)))
        {
            if(os.encoder2Count>COUNT_MIN)
                --os.encoder2Count;
        }
        //Pushbutton pressed
        if(ENCODER2_PB_PIN && (!(enc2&ENCODER2_PB_MASK)))
        {
            os.button2 = 1;
        }
        //Pushbutton released
        if((!ENCODER2_PB_PIN) && (enc2&ENCODER2_PB_MASK))
        {
            os.button2 = -1;
        }
        //Save current state
        enc2 = ENCODER2_PORT & ENCODER2_MASK; 
    }
}

void encoder_statemachine(void)
{
    //Immediately return if there is no user input
    if(os.encoder1Count==0 && os.encoder2Count==0 && os.button1==0 && os.button2==0)
    {
        return;
    }
    
    //React according to user input
    switch(os.displayState & 0xF0)
    {
        case DISPLAY_STATE_MAIN:
            switch(os.displayState)
            {
                case DISPLAY_STATE_MAIN_SETUP:
                    if(os.button1==1 || os.button2==1)
                        os.displayState = DISPLAY_STATE_SETUP1_CONFIRM;
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MAIN_DIVIDE;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MAIN_ZERO;
                    break;
                case DISPLAY_STATE_MAIN_DIVIDE:
                    if(os.button1==1 || os.button2==1)
                        os.displayState = DISPLAY_STATE_DIVIDE1_CONFIRM;
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MAIN_ARC;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MAIN_SETUP;
                    break;
                case DISPLAY_STATE_MAIN_ARC:
                    if(os.button1==1 || os.button2==1)
                        os.displayState = DISPLAY_STATE_ARC1_CONFIRM;
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MAIN_MANUAL;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MAIN_DIVIDE;
                    break;
                case DISPLAY_STATE_MAIN_MANUAL:
                    if(os.button1==1 || os.button2==1)
                        os.displayState = DISPLAY_STATE_MANUAL_CANCEL;
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MAIN_ZERO;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MAIN_ARC;
                    break;
                case DISPLAY_STATE_MAIN_ZERO:
                    if(os.button1==1 || os.button2==1)
                        os.displayState = DISPLAY_STATE_ZERO_NORMAL;
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MAIN_SETUP;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MAIN_MANUAL;
                    break;
                case DISPLAY_STATE_ENCODER_TEST:
                    //return; //i.e. not zeroing results
                    break;
            }
            break;

        case DISPLAY_STATE_SETUP1:
            switch(os.displayState)
            {
                case DISPLAY_STATE_SETUP1_CONFIRM:
                    if(os.button1==1)
                    {
                        //to do: set current position as zero position
                        os.displayState = DISPLAY_STATE_SETUP2_CCW;
                    }
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_SETUP1_CANCEL;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_SETUP1_CANCEL;
                    break;
                case DISPLAY_STATE_SETUP1_CANCEL:
                    if(os.button1==1)
                        os.displayState = DISPLAY_STATE_MAIN_SETUP;
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_SETUP1_CONFIRM;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_SETUP1_CONFIRM;
                    break;
            }
            if(os.button2==1)
            {
                os.setup_step_size = encoder_next_setup_stepsize(os.setup_step_size);
            }
            if(os.encoder2Count>0)
            {
                //Todo: increase position by step size
            }
            if(os.encoder2Count<0)
            {
                //Todo: decrease position by step size
            }
            break;

        case DISPLAY_STATE_SETUP2:
            switch(os.displayState)
            {
                case DISPLAY_STATE_SETUP2_CCW:
                    if(os.button1==1 || os.button2==1)
                    {
                        os.approach_direction = MOTOR_DIRECTION_CCW;
                        os.displayState = DISPLAY_STATE_MAIN_SETUP;
                    }
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_SETUP2_CW;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_SETUP2_CW;
                    break;
                case DISPLAY_STATE_SETUP2_CW:
                    if(os.button1==1 || os.button2==1)
                    {
                        os.approach_direction = MOTOR_DIRECTION_CCW;
                        os.displayState = DISPLAY_STATE_MAIN_SETUP;
                    }
                    if(os.encoder1Count+os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_SETUP2_CCW;
                    if(os.encoder1Count+os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_SETUP2_CCW;
                    break;
            }
            break;

        case DISPLAY_STATE_DIVIDE1:
            switch(os.displayState)
            {
                case DISPLAY_STATE_DIVIDE1_CONFIRM:
                    if(os.button1==1)
                    {
                        //to do: set division
                        os.displayState = DISPLAY_STATE_DIVIDE2_NORMAL;
                    }
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_DIVIDE1_CANCEL;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_DIVIDE1_CANCEL;
                    break;
                case DISPLAY_STATE_DIVIDE1_CANCEL:
                    if(os.button1==1)
                        os.displayState = DISPLAY_STATE_MAIN_DIVIDE;
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_DIVIDE1_CONFIRM;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_DIVIDE1_CONFIRM;
                    break;
            }
            if(os.button2==1)
            {
                os.divide_step_size = encoder_next_divide_step_size(os.divide_step_size);
            }
            if(os.encoder2Count>0)
            {
                os.division += os.divide_step_size;
            }
            if(os.encoder2Count<0)
            {
                if(os.division>os.divide_step_size)
                {
                    os.division -= os.divide_step_size;
                }
                else
                {
                    os.division = 1;
                }
            }
            break;

        case DISPLAY_STATE_DIVIDE2:
            if(os.button1==1)
                os.displayState = DISPLAY_STATE_MAIN_DIVIDE;
            if(os.button2==1)
            {
                //Todo: jump by jump size
                if(os.current_position+os.divide_jump_size<0)
                    os.current_position += os.division;
                os.current_position += os.divide_jump_size;
                os.current_position %= os.division;
            }
            if(os.encoder2Count>0)
            {
                ++os.divide_jump_size;
                if(os.divide_jump_size==0)
                    os.divide_jump_size = 1;
                if(os.divide_jump_size>=os.division)
                    os.divide_jump_size = os.division - 1;
            }
            if(os.encoder2Count<0)
            {
                --os.divide_jump_size;
                if(os.divide_jump_size==0)
                    os.divide_jump_size = -1;
                if(os.divide_jump_size<=(-os.division))
                    os.divide_jump_size = 1 - os.division;
            }
            break;

        case DISPLAY_STATE_ARC1:
            switch(os.displayState)
            {
                case DISPLAY_STATE_ARC1_CONFIRM:
                    if(os.button1==1)
                    {
                        os.displayState = DISPLAY_STATE_ARC2_CANCEL;
                    }
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_ARC1_CANCEL;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_ARC1_CANCEL;
                    break;
                case DISPLAY_STATE_ARC1_CANCEL:
                    if(os.button1==1)
                        os.displayState = DISPLAY_STATE_MAIN_ARC;
                    if(os.encoder1Count>0)
                        os.displayState = DISPLAY_STATE_ARC1_CONFIRM;
                    if(os.encoder1Count<0)
                        os.displayState = DISPLAY_STATE_ARC1_CONFIRM;
                    break;  
            }
            if(os.button2==1)
            {
                os.arc_step_size =  encoder_next_arc_step_size(os.arc_step_size);
            }
            if(os.encoder2Count>0)
            {
                os.arc_size += os.arc_step_size;
                if(os.arc_size>100000)
                    os.arc_size = 100000;
            }
            if(os.encoder2Count<0)
            {
                os.arc_size -= os.arc_step_size;
                if(os.arc_size<-100000)
                    os.arc_size = -100000;
            }
            break;

        case DISPLAY_STATE_ARC2:
            if(os.encoder1Count>0)
            {
                if(os.arc_speed<100) //Todo: define some max speed)
                    ++os.arc_speed;
            }
            if(os.encoder1Count<0)
            {
                if(os.arc_speed>1)
                    --os.arc_speed;
            }
            switch(os.displayState)
            {
                case DISPLAY_STATE_ARC2_CCW:
                    if(os.button2==1)
                    {
                        motor_run(MOTOR_DIRECTION_CCW, os.arc_size);
                    }
                    if(os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_ARC2_CANCEL;
                    break;
                case DISPLAY_STATE_ARC2_CANCEL:
                    if(os.button2==1)
                        os.displayState = DISPLAY_STATE_MAIN_ARC;
                    if(os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_ARC2_CW;
                    if(os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_ARC2_CCW;
                    break;
                case DISPLAY_STATE_ARC2_CW:
                    if(os.button2==1)
                    {
                        motor_run(MOTOR_DIRECTION_CW, os.arc_size);
                    }
                    if(os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_ARC2_CANCEL;
                    break;
            }
            break;

        case DISPLAY_STATE_ZERO:
            if(os.encoder1Count>0)
            {
                if(os.arc_speed<100) //Todo: define some max speed)
                    ++os.arc_speed;
            }
            if(os.encoder1Count<0)
            {
                if(os.arc_speed>1)
                    --os.arc_speed;
            }
            if(os.button2==1)
            {
                //Todo: drive to zero
                os.displayState = DISPLAY_STATE_MAIN_ZERO;
            }
            if(os.button1==1)
                os.displayState = DISPLAY_STATE_MAIN_ZERO;  
            break;

        case DISPLAY_STATE_MANUAL:
            if(os.encoder1Count>0)
            {
                if(os.manual_speed<100) //Todo: define some max speed)
                    ++os.manual_speed;
            }
            if(os.encoder1Count<0)
            {
                if(os.manual_speed>1)
                    --os.manual_speed;
            }
            switch(os.displayState)
            {
                case DISPLAY_STATE_MANUAL_CCW:
                    if(os.button2==1)
                    {
                        //to do: turn ccw
                    }
                    if(os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MANUAL_CANCEL;
                    break;
                case DISPLAY_STATE_MANUAL_CANCEL:
                    if(os.button2==1)
                        os.displayState = DISPLAY_STATE_MAIN_MANUAL;
                    if(os.encoder2Count>0)
                        os.displayState = DISPLAY_STATE_MANUAL_CW;
                    if(os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MANUAL_CCW;
                    break;
                case DISPLAY_STATE_MANUAL_CW:
                    if(os.button2==1)
                    {
                        //to do: turn cw
                    }
                    if(os.encoder2Count<0)
                        os.displayState = DISPLAY_STATE_MANUAL_CANCEL;
                    break;
            }
            break;
    }
    
    //Reset everything
    os.encoder1Count = 0;
    os.encoder2Count = 0;
    os.button1 = 0;
    os.button2 = 0;      
}


uint16_t encoder_next_setup_stepsize(uint16_t old_stepsize)
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

uint8_t encoder_next_divide_step_size(uint8_t old_stepsize)
{
    switch(old_stepsize)
    {
        case 1:
            return 10;
        case 10:
            return 100;
        case 100:
            return 1;
        default:
            return 1;
    }
}

uint16_t encoder_next_arc_step_size(uint16_t old_stepsize)
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
            return 1;
    }
}