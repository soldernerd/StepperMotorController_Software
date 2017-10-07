/* 
 * File:   system.h
 * Author: Luke
 *
 * Created on 5. September 2016, 21:17
 */

#ifndef OS_H
#define	OS_H

#include <stdint.h>

/*
 * General definitions
 */

#define _XTAL_FREQ 8000000

#define NUMBER_OF_TIMESLOTS 16

#define  PPSUnLock()    {EECON2 = 0b01010101; EECON2 = 0b10101010; PPSCONbits.IOLOCK = 0;}
#define  PPSLock() 		{EECON2 = 0b01010101; EECON2 = 0b10101010; PPSCONbits.IOLOCK = 1;}

#define PPS_FUNCTION_CCP1_OUTPUT 14
#define PPS_FUNCTION_CCP2_OUTPUT 18

//#define STEPPER_ENABLE_PIN TRISBbits.TRISB3
//#define STEPPER_DIRECTION_PIN PORTBbits.RB2

#define PIN_INPUT           1
#define PIN_OUTPUT          0
#define PIN_DIGITAL         1
#define PIN_ANALOG          0



#define TEMPERATURE_TRIS TRISAbits.TRISA0
#define TEMPERATURE_ANCON ANCON0bits.PCFG0

#define BUZZER_TRIS TRISAbits.TRISA1
#define BUZZER_PIN LATAbits.LATA1

#define FAN_TRIS TRISBbits.TRISB7
#define FAN_PIN LATBbits.LATB7

#define DISPLAY_RESET_TRIS TRISBbits.TRISB6
#define DISPLAY_RESET_PIN LATBbits.LATB6

#define DISPLAY_BACKLIGHT_TRIS TRISCbits.TRISC7
#define DISPLAY_BACKLIGHT_PIN LATCbits.LATC7
#define DISPLAY_BACKLIGHT_PPS RPOR18

#define I2C_SDA_TRIS TRISBbits.TRISB5
#define I2C_SCL_TRIS TRISBbits.TRISB4

#define MOTOR_ENABLE_TRIS TRISBbits.TRISB3
#define MOTOR_ENABLE_PIN LATBbits.LATB3

#define MOTOR_DIRECTION_TRIS TRISBbits.TRISB2
#define MOTOR_DIRECTION_PIN LATBbits.LATB2

#define MOTOR_STEP_TRIS TRISBbits.TRISB1
#define MOTOR_STEP_PIN LATBbits.LATB1
#define MOTOR_STEP_PPS RPOR4

#define MOTOR_ERROR_TRIS TRISBbits.TRISB0
#define MOTOR_ERROR_PIN PORTBbits.RB0
#define MOTOR_ERROR_ANCON ANCON1bits.PCFG12

#define ENCODER1_A_TRIS TRISCbits.TRISC2
#define ENCODER1_A_PIN PORTCbits.RC2
#define ENCODER1_A_ANCON ANCON1bits.PCFG11

#define ENCODER1_B_TRIS TRISCbits.TRISC0
#define ENCODER1_B_PIN PORTCbits.RC0
#define ENCODER1_B_ANCON NONE

#define ENCODER1_PB_TRIS TRISCbits.TRISC1
#define ENCODER1_PB_PIN PORTCbits.RC1
#define ENCODER1_PB_ANCON NONE

#define ENCODER2_A_TRIS TRISAbits.TRISA2
#define ENCODER2_A_PIN PORTAbits.RA2
#define ENCODER2_A_ANCON ANCON0bits.PCFG2

#define ENCODER2_B_TRIS TRISAbits.TRISA5
#define ENCODER2_B_PIN PORTAbits.RA5
#define ENCODER2_B_ANCON ANCON0bits.PCFG4

#define ENCODER2_PB_TRIS TRISAbits.TRISA3
#define ENCODER2_PB_PIN PORTAbits.RA3
#define ENCODER2_PB_ANCON ANCON0bits.PCFG3


/*
 * Type definitions
 */

typedef struct
{
    volatile uint8_t timeSlot;
    volatile uint8_t done;
    int16_t adc_values[16];
    int32_t adc_sum;
    int16_t db_value;
    uint8_t s_value;
    uint8_t s_fraction;
    int32_t calibration[14]; 
} os_t;


/*
 * Global variables
 */

os_t os;


/*
 * Function prototypes
 */


void tmr_isr(void);
void system_init(void);

#endif	/* OS_H */

