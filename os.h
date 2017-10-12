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

#define ENCODER1_PORT PORTC
#define ENCODER1_MASK 0b00000111

#define ENCODER1_A_TRIS TRISCbits.TRISC2
#define ENCODER1_A_PIN PORTCbits.RC2
#define ENCODER1_A_ANCON ANCON1bits.PCFG11
#define ENCODER1_A_MASK 0b00000100

#define ENCODER1_B_TRIS TRISCbits.TRISC0
#define ENCODER1_B_PIN PORTCbits.RC0
#define ENCODER1_B_ANCON NONE
#define ENCODER1_B_MASK 0b00000001

#define ENCODER1_PB_TRIS TRISCbits.TRISC1
#define ENCODER1_PB_PIN PORTCbits.RC1
#define ENCODER1_PB_ANCON NONE
#define ENCODER1_PB_MASK 0b00000010

#define ENCODER2_PORT PORTA
#define ENCODER2_MASK 0b00101100

#define ENCODER2_A_TRIS TRISAbits.TRISA2
#define ENCODER2_A_PIN PORTAbits.RA2
#define ENCODER2_A_ANCON ANCON0bits.PCFG2
#define ENCODER2_A_MASK 0b00000100

#define ENCODER2_B_TRIS TRISAbits.TRISA5
#define ENCODER2_B_PIN PORTAbits.RA5
#define ENCODER2_B_ANCON ANCON0bits.PCFG4
#define ENCODER2_B_MASK 0b00100000

#define ENCODER2_PB_TRIS TRISAbits.TRISA3
#define ENCODER2_PB_PIN PORTAbits.RA3
#define ENCODER2_PB_ANCON ANCON0bits.PCFG3
#define ENCODER2_PB_MASK 0b00001000


/*
 * Type definitions
 */

typedef enum 
{
    DISPLAY_STATE_MAIN = 0x00,
    DISPLAY_STATE_MAIN_SETUP = 0x01,
    DISPLAY_STATE_MAIN_DIVIDE = 0x02,
    DISPLAY_STATE_MAIN_ARC = 0x03,
    DISPLAY_STATE_MAIN_MANUAL = 0x04,
    DISPLAY_STATE_MAIN_ZERO = 0x05,
    DISPLAY_STATE_SETUP1 = 0x10,
    DISPLAY_STATE_SETUP1_CONFIRM = 0x11,
    DISPLAY_STATE_SETUP1_CANCEL = 0x12,        
    DISPLAY_STATE_SETUP2 = 0x20,
    DISPLAY_STATE_SETUP2_CCW = 0x21,
    DISPLAY_STATE_SETUP2_CW = 0x22,        
    DISPLAY_STATE_DIVIDE1 = 0x30,
    DISPLAY_STATE_DIVIDE1_CONFIRM = 0x31,
    DISPLAY_STATE_DIVIDE1_CANCEL = 0x32,
    DISPLAY_STATE_DIVIDE2 = 0x40,
    DISPLAY_STATE_DIVIDE2_NORMAL = 0x41,
    DISPLAY_STATE_ARC1 = 0x50,
    DISPLAY_STATE_ARC1_CONFIRM = 0x51,
    DISPLAY_STATE_ARC1_CANCEL = 0x52,
    DISPLAY_STATE_ARC2 = 0x60,
    DISPLAY_STATE_ARC2_CCW = 0x61,
    DISPLAY_STATE_ARC2_CANCEL = 0x62,
    DISPLAY_STATE_ARC2_CW = 0x63,
    DISPLAY_STATE_ZERO = 0x70,
    DISPLAY_STATE_ZERO_NORMAL = 0x71,
    DISPLAY_STATE_MANUAL = 0x80,
    DISPLAY_STATE_MANUAL_CCW = 0x81,
    DISPLAY_STATE_MANUAL_CANCEL = 0x82,
    DISPLAY_STATE_MANUAL_CW = 0x83,
    DISPLAY_STATE_ENCODER_TEST = 0xF0
} displayState_t;

typedef struct
{
    volatile uint8_t subTimeSlot;
    volatile uint8_t timeSlot;
    volatile uint8_t done;
    volatile int8_t encoder1Count;
    volatile int8_t button1;
    volatile int8_t encoder2Count;
    volatile int8_t button2;
    displayState_t displayState;
    uint8_t busy;
    int8_t last_approach_direction;
    uint16_t setup_step_size;
    int8_t approach_direction;
    uint16_t division;
    uint8_t divide_step_size;
    volatile uint16_t current_position;
    int16_t divide_jump_size;
    uint16_t arc_step_size;
    int16_t arc_size;
    uint8_t arc_speed;
    int8_t arc_direction;
    uint8_t manual_speed;
    int8_t manual_direction;
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

