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

const uint32_t pwm_frequency = 20000UL; // 20 kHz
const uint32_t max_duty = 10000UL;  // 100 %
const uint32_t min_duty = 4000UL;   // 50 %
const uint16_t max_rpm = 0x35UL;
const uint16_t min_rpm = 0x12UL;

typedef enum {
	COMMAND_START, COMMAND_STOP
} MotorCommandType_t;

typedef struct {
	MotorCommandType_t type;
	MotorParameters_t parameters;
	uint32_t crc;
} MotorCommand_t;

typedef enum {
	STATUS_STOPPED, STATUS_RUNNING
} MotorStatus_t;

StaticQueue_t xMotorCommandsStaticQueue;
uint8_t ucMotorCommandsQueueStorageArea[COMMAND_QUEUE_LENGTH * COMMAND_QUEUE_ITEM_SIZE];
QueueHandle_t xQueue_MotorCommands;

TaskHandle_t xMotorManagerHandle = NULL;
StaticTask_t xMotorManagerBuffer;
StackType_t xMotorManagerStack[300U];

TaskHandle_t xSpeedControllerHandle = NULL;
StaticTask_t xSpeedControllerBuffer;
StackType_t xSpeedControllerStack[300U];

SemaphoreHandle_t xStatusSemaphore = NULL;
StaticSemaphore_t xStatusMutexBuffer;

MotorStatus_t motor_status;
MotorParameters_t target_params;
uint32_t current_duty_cycle = 100UL;
uint16_t actual_rpm = 0U;
uint16_t requested_rpm_changed;

uint32_t generate_crc(MotorCommand_t *command)
{
	CRC_SW_CalculateCRC(&CRC_SW_0, command, sizeof (MotorCommand_t));

	return CRC_SW_GetCRCResult(&CRC_SW_0);
}

BaseType_t MotorManager_GetRPM(void)
{
	return actual_rpm;
}

BaseType_t MotorManager_SetSpeed(MotorSpeed_t rpm, MotorDirection_t direction)
{
	MotorCommand_t command;

	command.type = COMMAND_START;
	command.parameters.rpm = rpm;
	command.parameters.direction = direction;
	command.crc = 0U;

	command.crc = generate_crc(&command);

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

BaseType_t MotorManager_Stop()
{
	MotorCommand_t command;

	command.type = COMMAND_STOP;
	command.parameters.rpm = 0U;
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
	motor_status = STATUS_STOPPED;
	target_params.direction = DIR_NONE;
	target_params.rpm = 0U;
	requested_rpm_changed = 0U;

	xStatusSemaphore = xSemaphoreCreateMutexStatic(&xStatusMutexBuffer);

	xQueue_MotorCommands = xQueueCreateStatic(COMMAND_QUEUE_LENGTH, COMMAND_QUEUE_ITEM_SIZE, ucMotorCommandsQueueStorageArea, &xMotorCommandsStaticQueue);
	xMotorManagerHandle = xTaskCreateStatic(MotorManager_Main, "MotorManager", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xMotorManagerStack, &xMotorManagerBuffer);
	xSpeedControllerHandle = xTaskCreateStatic(MotorManager_SpeedController, "SpeedController", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xSpeedControllerStack, &xSpeedControllerBuffer);
}

void set_motor_direction(MotorDirection_t direction)
{
	switch (direction) {
	case DIR_FORWARD:
		DIGITAL_IO_SetOutputLow(&DIGITAL_IO_MotorDirection);
		break;
	case DIR_BACKWARD:
		DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_MotorDirection);
		break;
	default:
		break;
	}

	if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 200) == pdTRUE) {
		target_params.direction = direction;
		xSemaphoreGive(xStatusSemaphore);
	}
}

void set_motor_speed(MotorSpeed_t rpm)
{
	BaseType_t duty_cycle = 0UL;		// Duty cycle in 1/10000

	if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 250) == pdTRUE) {
		PWM_Stop(&PWM_Motor);

		if (0U == rpm) {
			duty_cycle = 0UL;
		} else if (rpm == target_params.rpm) {
			duty_cycle = current_duty_cycle;
		} else if (min_rpm >= rpm) {
			duty_cycle = min_duty;
		} else if (max_rpm <= rpm) {
			duty_cycle = max_duty;
		} else {
			uint32_t percentage = ((rpm - min_rpm) * 100U) / (max_rpm - min_rpm);

			duty_cycle = min_duty + ((max_duty - min_duty) * percentage) / 100UL;
		}

		if (duty_cycle > 0UL) {
			PWM_SetFreqAndDutyCycle(&PWM_Motor, pwm_frequency, duty_cycle);
			PWM_Start(&PWM_Motor);

			DIGITAL_IO_SetOutputLow(&DIGITAL_IO_MotorDisable);
			current_duty_cycle = duty_cycle;
			target_params.rpm = rpm;
		} else {
			DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_MotorDisable);
		}

		requested_rpm_changed = 1U;
		xSemaphoreGive(xStatusSemaphore);
	}
}

void MotorManager_SpeedController(void *pvParameters)
{
	const TickType_t xInterval = 333 / portTICK_PERIOD_MS;
	uint16_t gear_carrier_count, rpm, rpmdiff;
	uint16_t target_rpm;
	BaseType_t duty_cycle, duty_cycle_correction;

	while (1U) {
		gear_carrier_count = COUNTER_GetCurrentCount(&COUNTER_WheelRevolution);
		rpm = gear_carrier_count * 3U; //(gear_carrier_count * 60) / 3;

		if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 50) == pdTRUE) {
			if (1U == requested_rpm_changed) {
				requested_rpm_changed = 0U;
			} else {
				actual_rpm = rpm;
				target_rpm = target_params.rpm;

				if (STATUS_RUNNING == motor_status && 0U != target_rpm) {
					duty_cycle = current_duty_cycle;

					if (rpm != target_rpm) {
						if (rpm > target_rpm) {
							rpmdiff = rpm - target_rpm;
						} else {
							rpmdiff = target_rpm - rpm;
						}

						uint32_t percentage = (rpmdiff * 100U) / target_rpm;
						duty_cycle_correction = (current_duty_cycle * percentage) / 100UL;

						if (rpm > target_rpm) {
							duty_cycle -= duty_cycle_correction;
						} else {
							duty_cycle += duty_cycle_correction;
						}

						if (duty_cycle > max_duty) {
							duty_cycle = max_duty;
						} else if (duty_cycle < min_duty) {
							duty_cycle = min_duty;
						}

						current_duty_cycle = duty_cycle;
						PWM_SetFreqAndDutyCycle(&PWM_Motor, pwm_frequency, duty_cycle);
					}
				}
			}

			xSemaphoreGive(xStatusSemaphore);
		}

		COUNTER_ResetCounter(&COUNTER_WheelRevolution);
		vTaskDelay(xInterval);
	}
}

void MotorManager_Main(void *pvParameters)
{
	MotorCommand_t command;

	set_motor_speed(0U);
	set_motor_direction(DIR_NONE);

	while (1U) {
		if (xQueueReceive(xQueue_MotorCommands, &command, (TickType_t) 2000)) {
			if (pdFALSE == MotorManager_VerifyCommand(&command)) {
				continue;
			}

			switch (command.type) {
			case COMMAND_START:
				COUNTER_ResetCounter(&COUNTER_WheelRevolution);
				COUNTER_Start(&COUNTER_WheelRevolution);

				set_motor_direction(command.parameters.direction);
				set_motor_speed(command.parameters.rpm);

				if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 100) == pdTRUE) {
					motor_status = STATUS_RUNNING;
					xSemaphoreGive(xStatusSemaphore);
				}

				break;
			case COMMAND_STOP:
			default:
				set_motor_speed(0U);
				set_motor_direction(DIR_NONE);

				COUNTER_Stop(&COUNTER_WheelRevolution);

				if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 100) == pdTRUE) {
					motor_status = STATUS_STOPPED;
					xSemaphoreGive(xStatusSemaphore);
				}

				break;
			}
		}
	}
}
