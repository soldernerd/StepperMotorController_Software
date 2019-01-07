/*
 * File:   main.c
 * Author: Luke
 *
 * Created on 19. July 2017, 22:41
 */

/** INCLUDES *******************************************************/

#include "system.h"
#include <xc.h>
#include <stdint.h>

#include "usb.h"
#include "usb_device_hid.h"
#include "usb_device_msd.h"
//
#include "internal_flash.h"
//
#include "app_device_custom_hid.h"
#include "app_device_msd.h"


//User defined code
#include "os.h"
#include "encoder.h"
#include "i2c.h"
#include "display.h"
#include "motor.h"

static void _calculate_adc_sum(void);
static void _calculate_db_value(void);
static void _calculate_s_value(void);

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
MAIN_RETURN main(void)
{ 
    uint8_t startup_timer;
    float tmp;
    //uint16_t stepper_count = 0;
    //uint8_t direction = 0;
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);

    USBDeviceInit();
    USBDeviceAttach();
    
    //This is a user defined function
    system_init();

    startup_timer = 0;

    
    while(1)
    {
        SYSTEM_Tasks();

        //Do this as often as possible
        APP_DeviceMSDTasks();

        if(!os.done)
        { 
            //Do this every time
            APP_DeviceCustomHIDTasks();
            //Take care of state machine
            encoder_statemachine();
            
            //Calculate position in 0.01 degrees
            tmp = (float) os.current_position_in_steps;
            tmp *= 36000;
            tmp /= os.full_circle_in_steps;
            tmp += 0.5; //Round correctly
            os.current_position_in_degrees = (uint16_t) tmp;
            if(os.current_position_in_degrees==36000)
            {
                //Due to rounding, this might happen under certain conditions...
                os.current_position_in_degrees = 0;
            }
                  
            
            display_prepare();
            display_update();
            //Run any pending motor commands
            motor_process_cue();
            
            //Take care of beep
            if(os.beep_count)
            {
                --os.beep_count;
                if(!os.beep_count)
                {
                    BUZZER_PIN = 0;
                }
            }
            
            //Run periodic tasks
            switch(os.timeSlot&0b00001111)
            {       
                case 0: 
                    break;
                case 8:
                    //PORTBbits.RB3 = 1;
                    break;
            }
            os.done = 1;
        }
    }//end while(1)
}//end main


/*******************************************************************************
 End of File
*/
