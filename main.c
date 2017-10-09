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
//#include "lcd.h"
#include "i2c.h"
#include "display.h"

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
    uint16_t stepper_count = 0;
    uint8_t direction = 0;
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
            //i2c_adc_start(I2C_ADC_RESOLUTION_14BIT, I2C_ADC_GAIN_1);
            APP_DeviceCustomHIDTasks();
            
            //Run periodic tasks
            switch(os.timeSlot&0b00001111)
            {       
                case 0: 
                    //PORTBbits.RB3 = 0
                    ++stepper_count;
                    if(stepper_count==10)
                    {
                        MOTOR_DIRECTION_PIN = 0;
                        //MOTOR_ENABLE_PIN = 0;//enable
                        T2CONbits.TMR2ON = 1;
                        os.displayState = DISPLAY_STATE_MANUAL_NORMAL;
                        display_prepare();
                        display_update();
                    }
                    if(stepper_count==50)
                    {
                        MOTOR_ENABLE_PIN = 1;//disable
                        T2CONbits.TMR2ON = 0;
                    }
                    if(stepper_count==60)
                    {
                        MOTOR_DIRECTION_PIN = 1;
                        //MOTOR_ENABLE_PIN = 0;//enable
                        T2CONbits.TMR2ON = 1;
                    }
                    if(stepper_count==100)
                    {
                        MOTOR_ENABLE_PIN = 1;//disable
                        T2CONbits.TMR2ON = 0;
                        stepper_count = 0;
                    }
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
