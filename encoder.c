#include <stdint.h>
#include <xc.h>
#include "encoder.h"
#include "os.h"

#define COUNT_MIN -128
#define COUNT_MAX 127

uint8_t enc1;
uint8_t enc2;

void encoder_init(void)
{
   enc1 = ENCODER1_PORT & ENCODER1_MASK; 
   enc2 = ENCODER2_PORT & ENCODER2_MASK; 
}

void encoder_run(void)
{
    if(enc1 != ENCODER1_PORT & ENCODER1_MASK)
    {
        //A rising while B low -> +
        if(ENCODER1_A_PIN && (!ENCODER1_B_PIN) && (!(enc1&ENCODER1_A_MASK)))
        {
            if(os.encoder1Count<COUNT_MAX)
                ++os.encoder1Count;
        }
        //B rising while A low -> -
        if(ENCODER1_B_PIN && (!ENCODER1_A_PIN) && (!(enc1&ENCODER1_B_MASK)))
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
    
    if(enc2 != ENCODER2_PORT & ENCODER2_MASK)
    {
        //A rising while B low -> +
        if(ENCODER2_A_PIN && (!ENCODER2_B_PIN) && (!(enc1&ENCODER2_A_MASK)))
        {
            if(os.encoder2Count<COUNT_MAX)
                ++os.encoder2Count;
        }
        //B rising while A low -> -
        if(ENCODER2_B_PIN && (!ENCODER2_A_PIN) && (!(enc1&ENCODER2_B_MASK)))
        {
            if(os.encoder2Count>COUNT_MIN)
                --os.encoder2Count;
        }
        //Pushbutton pressed
        if(ENCODER2_PB_PIN && (!(enc1&ENCODER2_PB_MASK)))
        {
            os.button2 = 1;
        }
        //Pushbutton released
        if((!ENCODER2_PB_PIN) && (enc1&ENCODER2_PB_MASK))
        {
            os.button2 = -1;
        }
        //Save current state
        enc2 = ENCODER2_PORT & ENCODER2_MASK; 
    }
}
