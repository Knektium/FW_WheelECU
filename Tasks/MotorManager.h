/*
 * MotorManager.h
 *
 *  Created on: 9 Sep 2018
 *      Author: Jack
 */

#ifndef TASKS_MOTORMANAGER_H_
#define TASKS_MOTORMANAGER_H_

typedef uint16_t MotorSpeed_t;

typedef enum {
	DIR_NONE = 0U,
	DIR_FORWARD = 1U,
	DIR_BACKWARD = 2U
} MotorDirection_t;

typedef struct {
	MotorSpeed_t rpm;
	MotorDirection_t direction;
} MotorParameters_t;

typedef enum {
	STATUS_STOPPED = 0U,
	STATUS_RUNNING = 1U
} MotorStatus_t;

extern TaskHandle_t xMotorManagerHandle;
extern TaskHandle_t xSpeedControllerHandle;

void MotorManager_Init(void);
void MotorManager_Main(void *pvParameters);
void MotorManager_SpeedController(void *pvParameters);

BaseType_t MotorManager_SetSpeed(MotorSpeed_t rpm, MotorDirection_t direction);
BaseType_t MotorManager_Stop(void);
void MotorManager_GetSpeed(MotorParameters_t *params);
MotorStatus_t MotorManager_GetStatus(void);

#endif /* TASKS_MOTORMANAGER_H_ */
