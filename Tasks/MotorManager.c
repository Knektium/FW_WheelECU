/*
 * MotorManager.c
 *
 *  Created on: 9 Sep 2018
 *      Author: Jack
 */
#include <DAVE.h>

#include "tasks.h"
#include "MotorManager.h"

#define MOTOR_MANAGER_STACK_DEPTH	300U

#define COMMAND_QUEUE_LENGTH		8U
#define COMMAND_QUEUE_ITEM_SIZE		sizeof (MotorCommand_t)

typedef enum {
	COMMAND_START, COMMAND_STOP
} MotorCommandType_t;

typedef struct {
	MotorCommandType_t type;
	MotorParameters_t parameters;
	uint32_t crc;
} MotorCommand_t;

StaticQueue_t xMotorCommandsStaticQueue;
uint8_t ucMotorCommandsQueueStorageArea[COMMAND_QUEUE_LENGTH * COMMAND_QUEUE_ITEM_SIZE];
QueueHandle_t xQueue_MotorCommands;

TaskHandle_t xMotorManagerHandle = NULL;
StaticTask_t xMotorManagerBuffer;
StackType_t xMotorManagerStack[300U];

uint32_t generate_crc(MotorCommand_t *command)
{
	CRC_SW_CalculateCRC(&CRC_SW_0, command, sizeof (MotorCommand_t));

	return CRC_SW_GetCRCResult(&CRC_SW_0);
}

BaseType_t MotorManager_SetSpeed(MotorSpeed_t speed, MotorDirection_t direction)
{
	MotorCommand_t command;

	command.type = COMMAND_START;
	command.parameters.speed = speed;
	command.parameters.direction = direction;
	command.crc = 0U;

	command.crc = generate_crc(&command);

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

BaseType_t MotorManager_Stop()
{
	MotorCommand_t command;

	command.type = COMMAND_STOP;
	command.parameters.speed = 0U;
	command.parameters.direction = 0U;
	command.crc = 0U;

	command.crc = generate_crc(&command);

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

BaseType_t MotorManager_VerifyCommand(MotorCommand_t *command)
{
	uint32_t crc;
	uint32_t old_crc;
	BaseType_t passed = pdFALSE;

	return pdTRUE;

	old_crc = command->crc;
	command->crc = 0U;

	crc = generate_crc(command);

	if (crc == old_crc) {
		passed = pdTRUE;
	}

	return passed;
}

void MotorManager_Init(void)
{
	xQueue_MotorCommands = xQueueCreateStatic(COMMAND_QUEUE_LENGTH, COMMAND_QUEUE_ITEM_SIZE, ucMotorCommandsQueueStorageArea, &xMotorCommandsStaticQueue);
	xMotorManagerHandle = xTaskCreateStatic(MotorManager_Main, "MotorManager", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xMotorManagerStack, &xMotorManagerBuffer);
}

void set_motor_state(MotorDirection_t direction)
{
	PWM_Stop(&PWM_MotorForward);
	PWM_Stop(&PWM_MotorBackward);

	switch (direction) {
	case MOTOR_FORWARD:
		PWM_Start(&PWM_MotorForward);
		break;
	case MOTOR_BACKWARD:
		PWM_Start(&PWM_MotorBackward);
		break;
	case MOTOR_STILL:
	default:
		break;
	}
}

void set_motor_speed(MotorSpeed_t speed)
{
	const uint32_t frequency = 20000UL; // 20 kHz
	BaseType_t duty_cycle = 0UL;		// Duty cycle in 1/10000

	switch (speed) {
	case SPEED_HIGH:
		duty_cycle = 10000UL;
		break;
	case SPEED_MEDIUM:
		duty_cycle = 7500UL;
		break;
	case SPEED_LOW:
		duty_cycle = 5000UL;
		break;
	default:
		duty_cycle = 0UL;
		break;
	}

	PWM_SetFreqAndDutyCycle(&PWM_MotorForward, frequency, duty_cycle);
	PWM_SetFreqAndDutyCycle(&PWM_MotorBackward, frequency, duty_cycle);
}

void MotorManager_Main(void *pvParameters)
{
	MotorCommand_t command;

	set_motor_state(MOTOR_STILL);

	while(1U) {
		if (xQueueReceive(xQueue_MotorCommands, &command, (TickType_t) 2000)) {
			if (pdFALSE == MotorManager_VerifyCommand(&command)) {
				continue;
			}

			switch (command.type) {
			case COMMAND_START:
				set_motor_speed(command.parameters.speed);
				set_motor_state(command.parameters.direction);

				COUNTER_ResetCounter(&COUNTER_WheelRevolution);
				COUNTER_Start(&COUNTER_WheelRevolution);

				break;
			case COMMAND_STOP:
			default:
				set_motor_state(MOTOR_STILL);

				COUNTER_Stop(&COUNTER_WheelRevolution);

				break;
			}
		}
	}
}