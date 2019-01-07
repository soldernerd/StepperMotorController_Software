/* 
 * File:   motor.h
 * Author: Luke
 *
 * Created on 12. September 2017, 21:26
 */

#ifndef MOTOR_H
#define	MOTOR_H

/*
#define POSITION_MULTIPLIER 17
#define POSITION_SHIFT 7
#define TIME_BASE 44739
#define FULL_STEP_MASK 0b1111
#define FULL_STEP_SHIFT 4
#define MILLISECONDS_START 11
#define STEPCOUNT_START 16
#define ACCELERATION_PERIOD 120
*/

#define OVERSHOOT 200

#define FULL_STEP_MASK 0b1111
#define FULL_STEP_SHIFT 4
//#define MAXIMUM_PWM_PERIOD 4080

//#define POSITION_MULTIPLIER 31
//#define POSITION_SHIFT 10
//#define TIME_BASE 200000
//#define MILLISECONDS_START 50
//#define ACCELERATION_PERIOD 349
#define MINIMUM_SPEED 1
#define MAXIMUM_SPEED 380
#define MAXIMUM_SPEED_ARC 305
#define MAXIMUM_SPEED_MANUAL 305

#define MOTOR_INVERSE_DIRECTION

 
typedef enum 
{
    MOTOR_DIRECTION_CCW = -1,
    MOTOR_DIRECTION_CW = 1
} motorDirection_t;

typedef enum
{
    //MOTOR_MODE_START,
    //MOTOR_MODE_RUN,
    //MOTOR_MODE_STOP,
    MOTOR_MODE_MANUAL,
    MOTOR_MODE_PWM
}motorMode_t;

typedef struct
{
    motorDirection_t direction;
    uint32_t distance;
    uint16_t speed;
} motorCommand_t;

#define MOTOR_COMMAND_CUE_SIZE 8
#define MOTOR_COMMAND_CUE_MASK 0b111

void motor_init(void);
void motor_isr(void);

//Debugging functions
motorMode_t motor_get_mode(void);
uint16_t motor_get_current_speed(void);
uint16_t motor_get_maximum_speed(void);

//For the display
uint16_t motor_speed_from_index(uint16_t speed_index);

void startup(void);
void motor_start(motorDirection_t direction);
void motor_stop(void);
void motor_change_speed(uint16_t new_speed);

//Main tools
uint8_t motor_schedule_command(motorDirection_t direction, uint32_t distance_in_steps, uint16_t speed);
void motor_process_cue(void);
uint8_t motor_items_in_cue(void);

#endif	/* MOTOR_H */

