
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "i2c.h"
#include "os.h"


#define DISPLAY_CC_VERTICALBAR_ADDRESS 0x00
#define DISPLAY_CC_VERTICALBAR_BIT_PATTERN {0b00000100,0b00000100,0b00000100,0b00000100,0b00000100,0b00000100,0b00000100,0b00000100}
const uint8_t bit_pattern_verticalbar[8] = DISPLAY_CC_VERTICALBAR_BIT_PATTERN;

#define DISPLAY_CC_DEGREE_ADDRESS 0x01
#define DISPLAY_CC_DEGREE_BIT_PATTERN {0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}
const uint8_t bit_pattern_degree[8] = DISPLAY_CC_DEGREE_BIT_PATTERN;

#define DISPLAY_CC_ae_ADDRESS 0x02
#define DISPLAY_CC_ae_BIT_PATTERN {0b00001010, 0b00000000, 0b00001110, 0b00000001, 0b00001111, 0b00010001, 0b00001111, 0b00000000}
const uint8_t bit_pattern_ae[8] = DISPLAY_CC_ae_BIT_PATTERN;

#define DISPLAY_STARTUP_0 {'*',' ',' ','S','t','e','p','p','e','r',' ','M','o','t','o','r',' ',' ',' ','*'}
#define DISPLAY_STARTUP_1 {'*',' ',' ',' ',' ','C','o','n','t','r','o','l','l','e','r',' ',' ',' ',' ','*'}
#define DISPLAY_STARTUP_2 {'*',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','*'}
#define DISPLAY_STARTUP_3 {'*',' ',' ','s','o','l','d','e','r','n','e','r','d','.','c','o','m',' ',' ','*'}
char display_content[4][20] = {DISPLAY_STARTUP_0, DISPLAY_STARTUP_1, DISPLAY_STARTUP_2, DISPLAY_STARTUP_3};
#define DISPLAY_MAIN_0 {'M','a','i','n',' ','M','e','n','u',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_MAIN_1 {' ','S','e','t','u','p',' ',' ',' ',' ','D','i','v','i','d','e',' ',' ',' ',' '}
#define DISPLAY_MAIN_2 {' ','A','r','c',' ',' ',' ',' ',' ',' ','M','a','n','u','a','l',' ',' ',' ',' '}
#define DISPLAY_MAIN_3 {' ','G','o','2','Z','e','r','o',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
const char dc_main[4][20] = {DISPLAY_MAIN_0, DISPLAY_MAIN_1, DISPLAY_MAIN_2, DISPLAY_MAIN_3};
#define DISPLAY_SETUP1_0 {'S','e','t','u','p',':',' ','S','e','t',' ','z','e','r','o',' ','p','o','s','.'}
#define DISPLAY_SETUP1_1 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_SETUP1_2 {'S','t','e','p',' ','s','i','z','e',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','o','n','f','i','r','m'}
#define DISPLAY_SETUP1_3 {' ','x','.','x','x',DISPLAY_CC_DEGREE_ADDRESS,' ',' ',' ',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','a','n','c','e','l',' '}
const char dc_setup1[4][20] = {DISPLAY_SETUP1_0, DISPLAY_SETUP1_1, DISPLAY_SETUP1_2, DISPLAY_SETUP1_3};
#define DISPLAY_SETUP2_0 {'S','e','t','u','p',':',' ','S','e','t',' ','d','i','r','e','c','t','i','o','n'}
#define DISPLAY_SETUP2_1 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_SETUP2_2 {' ','C','o','u','n','t','e','r','C','l','o','c','k','w','i','s','e',' ',' ',' '}
#define DISPLAY_SETUP2_3 {' ','C','l','o','c','k','w','i','s','e',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
const char dc_setup2[4][20] = {DISPLAY_SETUP2_0, DISPLAY_SETUP2_1, DISPLAY_SETUP2_2, DISPLAY_SETUP2_3};
#define DISPLAY_DIVIDE1_0 {'D','i','v','i','d','e',':',' ','S','e','t',' ','d','i','v','i','s','i','o','n'}
#define DISPLAY_DIVIDE1_1 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_DIVIDE1_2 {'1','0','0','/','s','t','e','p',' ',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','o','n','f','i','r','m'}
#define DISPLAY_DIVIDE1_3 {'1','2','3','4',' ',' ',' ',' ',' ',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','a','n','c','e','l',' '}
const char dc_divide1[4][20] = {DISPLAY_DIVIDE1_0, DISPLAY_DIVIDE1_1, DISPLAY_DIVIDE1_2, DISPLAY_DIVIDE1_3};
#define DISPLAY_DIVIDE2_0 {'D','i','v','i','d','e',':',' ','1','2','3','4',',',' ','C','C','W',' ',' ',' '}
#define DISPLAY_DIVIDE2_1 {'C','u','r','r','e','n','t',' ','p','o','s',':',' ','1','2','3','4',' ',' ',' '}
#define DISPLAY_DIVIDE2_2 {'J','u','m','p',' ','s','i','z','e',':',' ','+','1','2','3',' ',' ',' ',' ',' '}
#define DISPLAY_DIVIDE2_3 {'P','r','e','s','s','T','o','J','u','m','p',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ','C','a','n','c','e','l'}
const char dc_divide2[4][20] = {DISPLAY_DIVIDE2_0, DISPLAY_DIVIDE2_1, DISPLAY_DIVIDE2_2, DISPLAY_DIVIDE2_3};
#define DISPLAY_ARC1_0 {'A','r','c',':',' ','S','e','t',' ','a','r','c',' ','s','i','z','e',' ',' ',' '}
#define DISPLAY_ARC1_1 {'A','r','c',' ','s','i','z','e',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_ARC1_2 {'S','t','e','p',' ','s','i','z','e',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','o','n','f','i','r','m'}
#define DISPLAY_ARC1_3 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',DISPLAY_CC_VERTICALBAR_ADDRESS,' ',' ','C','a','n','c','e','l',' '}
const char dc_arc1[4][20] = {DISPLAY_ARC1_0, DISPLAY_ARC1_1, DISPLAY_ARC1_2, DISPLAY_ARC1_3};
#define DISPLAY_ARC2_0 {'A','r','c',':',' ','S','i','z','e','=',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_ARC2_1 {'C','u','r','r','e','n','t',' ','p','o','s',':',' ','1','2','3','.','4','5','°'}
#define DISPLAY_ARC2_2 {'T','u','r','n',' ','C','C','W',' ','|',' ','S','p','e','e','d',' ',' ',' ',' '}
#define DISPLAY_ARC2_3 {'S','t','a','r','t',' ',' ',' ',' ','|',' ',' ','2','.','3','4','°','/','s',' '}
const char dc_arc2[4][20] = {DISPLAY_ARC2_0, DISPLAY_ARC2_1, DISPLAY_ARC2_2, DISPLAY_ARC2_3};
#define DISPLAY_ZERO_0 {'R','e','t','u','r','n',' ','t','o',' ','Z','e','r','o','?',' ',' ',' ',' ',' '}
#define DISPLAY_ZERO_1 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_ZERO_2 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_ZERO_3 {' ','Y','e','s',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','N','o',' ',' '}
const char dc_zero[4][20] = {DISPLAY_ZERO_0, DISPLAY_ZERO_1, DISPLAY_ZERO_2, DISPLAY_ZERO_3};
#define DISPLAY_MANUAL_0 {'M','a','n','u','a','l',' ','M','o','d','e',' ',' ',' ',' ',' ',' ',' ',' ',' '}
#define DISPLAY_MANUAL_1 {'C','u','r','r','e','n','t',' ','p','o','s',':',' ',' ',' ','3','.','4','5','°'}
#define DISPLAY_MANUAL_2 {'T','u','r','n',' ','C','C','W',' ','|',' ','S','p','e','e','d',' ',' ',' ',' '}
#define DISPLAY_MANUAL_3 {'S','t','a','r','t',' ',' ',' ',' ','|',' ','1','2','.','3','4','°','/','s',' '}
const char dc_manual[4][20] = {DISPLAY_MANUAL_0, DISPLAY_MANUAL_1, DISPLAY_MANUAL_2, DISPLAY_MANUAL_3};

static void _display_clear(void);
static void _display_itoa(int16_t value, uint8_t decimals, char *text);

static void _display_clear(void)
{
    uint8_t row;
    uint8_t col;
    for(row=0;row<4;++row)
    {
        for(col=0;col<20;++col)
        {
            display_content[row][col] = ' ';
        }
    }
}

static void _display_itoa(int16_t value, uint8_t decimals, char *text)
{
    uint8_t pos;
    uint8_t len;
    int8_t missing;
    char tmp[10];
    itoa(tmp, value, 10);
    len = strlen(tmp);
    
    if(value<0) //negative values
    {
        missing = decimals + 2 - len;
        if(missing>0) //zero-padding needed
        {
            for(pos=decimals;pos!=0xFF;--pos)
            {
                if(pos>=missing) //there is a character to copy
                {
                    tmp[pos+1] = tmp[pos+1-missing];
                }
                else //there is no character
                {
                    tmp[pos+1] = '0';
                }
            }
            len = decimals + 2;
        }  
    }
    else
    {
        missing = decimals + 1 - len;
        if(missing>0) //zero-padding needed
        {
            for(pos=decimals;pos!=0xFF;--pos)
            {
                if(pos>=missing) //there is a character to copy
                {
                    tmp[pos] = tmp[pos-missing];
                }
                else //there is no character
                {
                    tmp[pos] = '0';
                }
            }
            len = decimals + 1;
        }       
    }
 
    decimals = len - decimals - 1;
    
    for(pos=0;pos<len;++pos)
    {
        text[pos] = tmp[pos];
        if(pos==decimals)
        {
            //Insert decimal point
            ++pos;
            text[pos] = '.';
            break;
        }
    }
    for(;pos<len;++pos)
    {
        text[pos+1] = tmp[pos];
    }
    text[pos+1] = 0;
}

void display_init(void)
{
    i2c_display_init();
    i2c_display_program_custom_character(DISPLAY_CC_VERTICALBAR_ADDRESS, bit_pattern_verticalbar); 
    i2c_display_program_custom_character(DISPLAY_CC_DEGREE_ADDRESS, bit_pattern_degree);
    i2c_display_program_custom_character(DISPLAY_CC_ae_ADDRESS, bit_pattern_ae);
}

void display_prepare()
{
    uint8_t cntr;
    uint8_t space;
    char temp[10];
    
    switch(os.displayState & 0xF0)
    {
        
        case DISPLAY_STATE_MAIN:
            memcpy(display_content, dc_main, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_MAIN_SETUP:
                    display_content[1][0] = '>';
                    break;
                case DISPLAY_STATE_MAIN_DIVIDE:
                    display_content[1][9] = '>';
                    break;
                case DISPLAY_STATE_MAIN_ARC:
                    display_content[2][0] = '>';
                    break;
                case DISPLAY_STATE_MAIN_MANUAL:
                    display_content[2][9] = '>';
                    break;
                case DISPLAY_STATE_MAIN_ZERO:
                    display_content[3][0] = '>';
                    break;
            }
            break;
            
        case DISPLAY_STATE_SETUP1:    
            memcpy(display_content, dc_setup1, sizeof display_content);
            _display_itoa(os.setup_step_size, 2, temp);
            if(os.setup_step_size>999)
                space = 0;
            else
                space = 1;
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[3][space+cntr] = temp[cntr];
            }
            switch(os.displayState)
            {
                case DISPLAY_STATE_SETUP1_CONFIRM:
                    display_content[2][12] = '>';
                    break;
                case DISPLAY_STATE_SETUP1_CANCEL:
                    display_content[3][12] = '>';
                    break;
            }
            break;
            
        case DISPLAY_STATE_SETUP2:    
            memcpy(display_content, dc_setup2, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_SETUP2_CCW:
                    display_content[2][0] = '>';
                    break;
                case DISPLAY_STATE_SETUP2_CW:
                    display_content[3][0] = '>';
                    break;
            }
            break;
            
        case DISPLAY_STATE_DIVIDE1:    
            memcpy(display_content, dc_divide1, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_DIVIDE1_CONFIRM:
                    display_content[2][12] = '>';
                    break;
                case DISPLAY_STATE_DIVIDE1_CANCEL:
                    display_content[3][12] = '>';
                    break;
            }
            break;
            
        case DISPLAY_STATE_DIVIDE2:    
            memcpy(display_content, dc_divide2, sizeof display_content);
            break;
            
        case DISPLAY_STATE_ARC1:    
            memcpy(display_content, dc_arc1, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_ARC1_CONFIRM:
                    display_content[2][12] = '>';
                    break;
                case DISPLAY_STATE_ARC1_CANCEL:
                    display_content[3][12] = '>';
                    break;
            }
            
            //Write arc size
            _display_itoa(os.arc_size, 2, temp);
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[1][10+cntr] = temp[cntr];
            }
            display_content[1][10+cntr] = '°';
            
            //Write arc step size
            _display_itoa(os.arc_step_size, 2, temp);
            if(os.arc_step_size>999)
                space = 0;
            else
                space = 1;
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[3][space+cntr] = temp[cntr];
            }
            display_content[3][space+cntr] = '°';
            
            break;
            
        case DISPLAY_STATE_ARC2:    
            memcpy(display_content, dc_arc2, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_ARC2_CW:
                    display_content[2][6] = 'W';
                    display_content[2][7] = ' ';
                    break;
                case DISPLAY_STATE_ARC2_CANCEL:
                    memcpy(display_content[2], "        ", 8);
                    memcpy(display_content[3], "Cancel", 6);
                    break;
            }
            
            //Write arc size
            _display_itoa(os.arc_size, 2, temp);
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[0][10+cntr] = temp[cntr];
            }
            display_content[0][10+cntr] = '°';
            
            //Write current position
            _display_itoa(os.current_position, 2, temp);
            if(os.current_position>9999)
                space = 0;
            else if(os.current_position>999)
                space = 1;
            else
                space = 2;
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[1][13+space+cntr] = temp[cntr];
            }
            break;
            
        case DISPLAY_STATE_ZERO:    
            memcpy(display_content, dc_zero, sizeof display_content);
            break;
            
        case DISPLAY_STATE_MANUAL:    
            memcpy(display_content, dc_manual, sizeof display_content);
            switch(os.displayState)
            {
                case DISPLAY_STATE_MANUAL_CW:
                    display_content[2][6] = 'W';
                    display_content[2][7] = ' ';
                    break;
                case DISPLAY_STATE_MANUAL_CANCEL:
                    memcpy(display_content[2], "        ", 8);
                    memcpy(display_content[3], "Cancel", 6);
                    break;
            }
            _display_itoa(os.current_position, 2, temp);
            if(os.current_position>9999)
                space = 0;
            else if(os.current_position>999)
                space = 1;
            else
                space = 2;
            for(cntr=0; temp[cntr]; ++cntr)
            {
                display_content[1][13+space+cntr] = temp[cntr];
            }
            break;
            
        case DISPLAY_STATE_ENCODER_TEST:
            _display_clear();
            _display_itoa((int16_t) (os.encoder1Count), 0, display_content[0]);
            if(ENCODER1_A_PIN)
                display_content[0][8] = 'H';
            else
                display_content[0][8] = 'L';
            if(ENCODER1_B_PIN)
                display_content[0][9] = 'H';
            else
                display_content[0][9] = 'L';
            _display_itoa((int16_t) (os.button1), 0, display_content[1]);
            if(ENCODER1_PB_PIN)
                display_content[1][8] = 'H';
            else
                display_content[1][8] = 'L';
            _display_itoa((int16_t) (os.encoder2Count), 0, display_content[2]);
            if(ENCODER2_A_PIN)
                display_content[2][8] = 'H';
            else
                display_content[2][8] = 'L';
            if(ENCODER2_B_PIN)
                display_content[2][9] = 'H';
            else
                display_content[2][9] = 'L';
            _display_itoa((int16_t) (os.button2), 0, display_content[3]);
            if(ENCODER2_PB_PIN)
                display_content[3][8] = 'H';
            else
                display_content[3][8] = 'L';
            break;
    }
    /*
    char buffer[10];
    uint8_t cntr;
    uint8_t offset;
    int16_t threshold;
    _display_clear();
    //Line 1
    
    
    if(os.db_value==-32768)
    {
        //There is no signal
        buffer[0] = '<';
        buffer[1] = '-';
        buffer[2] = '1';
        buffer[3] = '2';
        buffer[4] = '0';
        buffer[5] = 'd';
        buffer[6] = 'B';
        buffer[7] = 'm';
        buffer[8] = 0;
        cntr = 0;
        while(buffer[cntr])
            lcd_content[0][cntr+4] = buffer[cntr++];        
    }
    else if(os.db_value==32767)
    {
        //Overload
        buffer[0] = '>';
        buffer[1] = '+';
        buffer[2] = '1';
        buffer[3] = '0';
        buffer[4] = 'd';
        buffer[5] = 'B';
        buffer[6] = 'm';
        buffer[7] = 0;
        cntr = 0;
        while(buffer[cntr])
            lcd_content[0][cntr+4] = buffer[cntr++];          
    }
    else
    {
        //Write db value
        offset = 2;
        if(os.db_value<0)
            offset--;
        if(os.db_value<-9999)
            offset--;
        if(os.db_value>0)
            lcd_content[0][offset-1] = '+';
        _display_itoa((int16_t) (os.db_value), 2, &buffer[0]);
        cntr = 0;
        while(buffer[cntr])
            lcd_content[0][offset+cntr] = buffer[cntr++];
        lcd_content[0][offset+cntr++] = 'd';
        lcd_content[0][offset+cntr++] = 'B';
        lcd_content[0][offset+cntr++] = 'm';
        
        //Write S value
        lcd_content[0][11] = 'S';
        _display_itoa((int16_t) (os.s_value), 0, &buffer[0]);
        lcd_content[0][12] = buffer[0];
        if(os.s_fraction!=0)
        {
            lcd_content[0][13] = '+';
            _display_itoa((int16_t) (os.s_fraction), 0, &buffer[0]);
            lcd_content[0][14] = buffer[0]; 
            if(os.s_fraction>9)
            {
                lcd_content[0][15] = buffer[1]; 
            }       
        }
    }
        
    if(os.db_value==-32768)
    {
        //There is no signal
        buffer[0] = 'N';
        buffer[1] = 'o';
        buffer[2] = ' ';
        buffer[3] = 'S';
        buffer[4] = 'i';
        buffer[5] = 'g';
        buffer[6] = 'n';
        buffer[7] = 'a';
        buffer[8] = 'l';
        buffer[9] = 0;
        cntr = 0;
        while(buffer[cntr])
            lcd_content[1][cntr+4] = buffer[cntr++];
    }
    else if(os.db_value==32767)
    {
        //Signal overload
        buffer[0] = 'O';
        buffer[1] = 'v';
        buffer[2] = 'e';
        buffer[3] = 'r';
        buffer[4] = 'l';
        buffer[5] = 'o';
        buffer[6] = 'a';
        buffer[7] = 'd';
        buffer[8] = 0;
        cntr = 0;
        while(buffer[cntr])
            lcd_content[1][cntr+5] = buffer[cntr++];
    }
    else
    {
        //Bar chart
        cntr = 0;
        threshold = -11340;
        while(os.db_value>threshold)
        {
            lcd_content[1][cntr] = LCD_CUSTOM_CHARACTER_BAR5;
            ++cntr;
            threshold += 825;
        }
        threshold = os.db_value - threshold + 825;
        if(threshold>660)
        {
            lcd_content[1][cntr] = LCD_CUSTOM_CHARACTER_BAR4;
        }
        else if(threshold>495)
        {
            lcd_content[1][cntr] = LCD_CUSTOM_CHARACTER_BAR3;
        }
        else if(threshold>330)
        {
            lcd_content[1][cntr] = LCD_CUSTOM_CHARACTER_BAR2;
        }
        else if(threshold>165)
        {
            lcd_content[1][cntr] = LCD_CUSTOM_CHARACTER_BAR1;
        }
    }
    */
}

void display_update(void)
{
    uint8_t line;
    for(line=0; line<4; ++line)
    {
        i2c_display_cursor(line, 0);
        i2c_display_write_fixed(display_content[line], 20);
    }
}

