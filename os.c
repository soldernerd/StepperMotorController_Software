
#include <stdint.h>
#include <xc.h>
#include "os.h"
#include "lcd.h"
#include "i2c.h"
//#include "adc.h"


//12ms for normal load, 1ms for short load
#define TIMER0_LOAD_HIGH_48MHZ 0xB9
#define TIMER0_LOAD_LOW_48MHZ 0xB0
#define TIMER0_LOAD_SHORT_HIGH_48MHZ 0xFA
#define TIMER0_LOAD_SHORT_LOW_48MHZ 0x24

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

static void display_init(void)
{
    //Setup backlight
    //pin as output
    TRISCbits.TRISC7 = 0;
    PORTCbits.RC7 = 0;
    
    //Assign via peripheral pin select (PPS)
    PPSUnLock();
    RPOR18 = 18;
    PPSLock();
    
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

static void stepper_init(void)
{
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
    
    //Initialize timer 2
    //Use timer2 for CCP1 module
    TCLKCONbits.T3CCP1 = 0;
    
    //Post scaler = 16
    T2CONbits.T2OUTPS = 0b1111;
    
    //Prescaler = 16
    T2CONbits.T2CKPS = 0b10;
    //T2CONbits.T2CKPS0 = 0;
    PR2 = 100;
    
    //Enable timer 2
    T2CONbits.TMR2ON = 1;
    
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
    init_buzzer();
    display_init();
    stepper_init();
    
    //Set up timer0 for timeSlots
    _system_timer0_init();
    
    //Set up I2C
    //i2c_init();

    //Set up LCD and display startup screen
    //lcd_setup();
    //lcd_init_4bit();
    //lcd_refresh_all(); 

}

