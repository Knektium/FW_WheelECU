/*
 * MotorManager.h
 *
 *  Created on: 9 Sep 2018
 *      Author: Jack
 */

#ifndef TASKS_MOTORMANAGER_H_
#define TASKS_MOTORMANAGER_H_

typedef enum {
	MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_STILL
} MotorDirection_t;

typedef enum {
	SPEED_HIGH, SPEED_MEDIUM, SPEED_LOW, SPEED_STILL
} MotorSpeed_t;

typedef struct {
	MotorSpeed_t speed;
	MotorDirection_t direction;
} MotorParameters_t;

extern TaskHandle_t xMotorManagerHandle;

void MotorManager_Init(void);
void MotorManager_Main(void *pvParameters);

BaseType_t MotorManager_SetSpeed(MotorSpeed_t speed, MotorDirection_t direction);
BaseType_t MotorManager_Stop();

#endif /* TASKS_MOTORMANAGER_H_ */
