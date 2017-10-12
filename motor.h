/* 
 * File:   motor.h
 * Author: Luke
 *
 * Created on 12. September 2017, 21:26
 */

#ifndef MOTOR_H
#define	MOTOR_H

#define POSITION_MULTIPLIER 17
#define POSITION_SHIFT 7
#define TIME_BASE 44739
#define FULL_STEP_MASK 0b1111
#define FULL_STEP_SHIFT 4
#define MILLISECONDS_START 11
#define STEPCOUNT_START 16
#define ACCELERATION_PERIOD 350

typedef enum 
{
    MOTOR_DIRECTION_CCW = -1,
    MOTOR_DIRECTION_CW = 1
} motorDirection_t;

void motor_init(void);
void motor_isr(void);
void motor_run(motorDirection_t direction, uint16_t distance);

#endif	/* MOTOR_H */

