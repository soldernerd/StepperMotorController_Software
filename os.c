
#include <stdint.h>
#include <xc.h>
#include "os.h"
//#include "lcd.h"
#include "i2c.h"
//#include "adc.h"


//12ms for normal load, 1ms for short load
#define TIMER0_LOAD_HIGH_48MHZ 0xB9
#define TIMER0_LOAD_LOW_48MHZ 0xB0
#define TIMER0_LOAD_SHORT_HIGH_48MHZ 0xFA
#define TIMER0_LOAD_SHORT_LOW_48MHZ 0x24

static void _system_pin_setup(void)
{
    TEMPERATURE_TRIS = PIN_INPUT;
    TEMPERATURE_ANCON = PIN_ANALOG;

    BUZZER_TRIS = PIN_OUTPUT;
    BUZZER_PIN = 0;

    FAN_TRIS = PIN_OUTPUT;
    FAN_PIN = 0;

    DISPLAY_RESET_TRIS = PIN_OUTPUT;
    DISPLAY_RESET_PIN = 0;

    DISPLAY_BACKLIGHT_TRIS = PIN_OUTPUT;
    DISPLAY_BACKLIGHT_PIN = 0;
    PPSUnLock();
    DISPLAY_BACKLIGHT_PPS = PPS_FUNCTION_CCP2_OUTPUT;
    PPSLock();

    I2C_SDA_TRIS = PIN_INPUT;
    I2C_SCL_TRIS = PIN_INPUT;

    MOTOR_ENABLE_TRIS = PIN_OUTPUT;
    MOTOR_ENABLE_PIN = 1;

    MOTOR_DIRECTION_TRIS = PIN_OUTPUT;
    MOTOR_DIRECTION_PIN = 0;

    MOTOR_STEP_TRIS = PIN_OUTPUT;
    MOTOR_STEP_PIN = 0;
    PPSUnLock();
    MOTOR_STEP_PPS = PPS_FUNCTION_CCP1_OUTPUT;
    PPSLock();

    MOTOR_ERROR_TRIS = PIN_INPUT;
    MOTOR_ERROR_ANCON = PIN_DIGITAL;

    ENCODER1_A_TRIS = PIN_INPUT;
    ENCODER1_A_ANCON = PIN_DIGITAL;

    ENCODER1_B_TRIS = PIN_INPUT;
    //ENCODER1_B_ANCON = PIN_DIGITAL;

    ENCODER1_PB_TRIS = PIN_INPUT;
    //ENCODER1_PB_ANCON = PIN_DIGITAL;

    ENCODER2_A_TRIS = PIN_INPUT;
    ENCODER2_A_ANCON = PIN_DIGITAL;

    ENCODER2_B_TRIS = PIN_INPUT;
    ENCODER2_B_ANCON = PIN_DIGITAL;

    ENCODER2_PB_TRIS = PIN_INPUT;
    ENCODER2_PB_ANCON = PIN_DIGITAL;
}

void tmr_isr(void)
{ 
    //Timer 0
    if(INTCONbits.T0IF)
    {
        if(os.done) 
        {
            //8ms until overflow
            TMR0H = TIMER0_LOAD_HIGH_48MHZ;
            TMR0L = TIMER0_LOAD_LOW_48MHZ;
            ++os.timeSlot;
            if(os.timeSlot==NUMBER_OF_TIMESLOTS)
            {
                os.timeSlot = 0;
            }
            os.done = 0;
        }
        else //Clock stretching
        {
            //1ms until overflow
            TMR0H = TIMER0_LOAD_SHORT_HIGH_48MHZ;
            TMR0L = TIMER0_LOAD_SHORT_LOW_48MHZ;
        }
        INTCONbits.T0IF = 0;
    }
}


static void _system_timer0_init(void)
{
    //Clock source = Fosc/4
    T0CONbits.T0CS = 0;
    //Operate in 16bit mode
    T0CONbits.T08BIT = 0;
    //Prescaler=8
    T0CONbits.T0PS2 = 0;
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS0 = 0;
    //Use prescaler
    T0CONbits.PSA = 0;
    //8ms until overflow
    TMR0H = TIMER0_LOAD_HIGH_48MHZ;
    TMR0L = TIMER0_LOAD_LOW_48MHZ;
    //Turn timer0 on
    T0CONbits.TMR0ON = 1;
            
    //Enable timer0 interrupts
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
    
    //Initialize timeSlot
    os.timeSlot = 0;
}

static void _backlight_init(void)
{
    //Initialize timer 4
    //Use timer2 for CCP1 module, timer 4 for CCP2 module
    TCLKCONbits.T3CCP2 = 0b0;
    TCLKCONbits.T3CCP1 = 0b1;
    
    //Prescaler = 1
    T4CONbits.T4CKPS = 0b00;
    
    //Enable timer 4
    T4CONbits.TMR4ON = 1;
    
    //Single output mode
    CCP2CONbits.P2M = 0b00;
    //PWM mode both outputs active high
    CCP2CONbits.CCP2M = 0b1100;
    //Duty cycle LSBs
    CCP2CONbits.DC2B = 0b00;
    //Duty cycle high MSBs
    CCPR2L = 150;
}

static void init_buzzer(void)
{
    TRISAbits.TRISA1 = 0;
    PORTAbits.RA1 = 1;
    __delay_ms(200);
    PORTAbits.RA1 = 0;
    __delay_ms(300);
    PORTAbits.RA1 = 1;
    __delay_ms(200);
    PORTAbits.RA1 = 0;
}

static void _stepper_init(void)
{
    /*
    //RB as input (error)
    TRISBbits.TRISB0 = 1;
    
    //RB3 as output (enable)
    TRISBbits.TRISB3 = 0;
    PORTBbits.RB3 = 0;
    
    //RB2 as output (direction)
    TRISBbits.TRISB2 = 0;
    PORTBbits.RB2 = 0;
    
    //RB1 as output (step)
    TRISBbits.TRISB1 = 0;
    PORTBbits.RB1 = 0;
    
    //Assign via peripheral pin select (PPS)
    PPSUnLock();
    RPOR4 = 14;
    PPSLock();
    */
    
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
    CCPR1L = PR2>>1;;
}

void system_init(void)
{
    //Configure all pins as inputs/outputs analog/digital as needed
    _system_pin_setup();
    
    //Set up I2C
    i2c_init();
    
    //Initialize display and show startup screen
    i2c_display_init();
    _backlight_init();
    
    //Configure timer2 and CCCP1 module to control stepper motor
    _stepper_init();

    //Set up timer0 for timeSlots
    _system_timer0_init();
}

