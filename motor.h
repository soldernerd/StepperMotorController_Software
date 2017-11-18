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

#define FULL_STEP_MASK 0b1111
#define FULL_STEP_SHIFT 4
//#define MAXIMUM_PWM_PERIOD 4080

//#define POSITION_MULTIPLIER 31
//#define POSITION_SHIFT 10
//#define TIME_BASE 200000
//#define MILLISECONDS_START 50
//#define ACCELERATION_PERIOD 349
#define MAXIMUM_SPEED 300
 
typedef enum 
{
    MOTOR_DIRECTION_CCW = -1,
    MOTOR_DIRECTION_CW = 1
} motorDirection_t;

typedef enum
{
    MOTOR_MODE_START,
    MOTOR_MODE_RUN,
    MOTOR_MODE_STOP
}motorMode_t;

void motor_init(void);
void motor_isr(void);
void motor_run(motorDirection_t direction, uint16_t distance);

//Debugging functions
void startup(void);
void motor_start(motorDirection_t direction);
void motor_stop(void);
void motor_change_speed(uint8_t new_speed);

//Some utilities




#endif	/* MOTOR_H */

