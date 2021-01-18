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

// Queues
StaticQueue_t xMotorCommandsStaticQueue;
uint8_t ucMotorCommandsQueueStorageArea[COMMAND_QUEUE_LENGTH * COMMAND_QUEUE_ITEM_SIZE];
QueueHandle_t xQueue_MotorCommands;

// Task handles, buffers and stacks
TaskHandle_t xMotorManager_MainTask_Handle = NULL;
StaticTask_t xMainTask_Buffer;
StackType_t xMainTask_Stack[300U];

TaskHandle_t xMotorManager_SpeedControllerTask_Handle = NULL;
StaticTask_t xSpeedControllerTask_Buffer;
StackType_t xSpeedControllerTask_Stack[300U];

TaskHandle_t xMotorManager_DiagnosticsTask_Handle = NULL;
StaticTask_t xDiagnosticsTask_Buffer;
StackType_t xDiagnosticsTask_Stack[300U];

// Semaphores
SemaphoreHandle_t xStatusSemaphore = NULL;
StaticSemaphore_t xStatusMutexBuffer;
SemaphoreHandle_t xDiagnosticsSemaphore = NULL;
StaticSemaphore_t xDiagnosticsMutexBuffer;

// Motor state
MotorStatus_t motor_status;
MotorParameters_t target_params;
MotorDiagnosis_t motor_diag;
uint32_t current_duty_cycle = 100UL;
uint16_t actual_rpm = 0U;
uint16_t requested_rpm_changed;

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

uint32_t generate_crc(MotorCommand_t *command)
{
	CRC_SW_CalculateCRC(&CRC_SW_0, command, sizeof (MotorCommand_t));

	return CRC_SW_GetCRCResult(&CRC_SW_0);
}

BaseType_t verify_command(MotorCommand_t *command)
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

uint8_t get_driver_diag()
{
	const uint8_t RD_DIA = 0x00U;
	const uint8_t RES_DIA = 0x80U;

	uint8_t spi_read_data;
	uint8_t spi_write_data;

	spi_read_data = 0xFFU;
	spi_write_data = RES_DIA;

	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);

	spi_write_data = RD_DIA;

	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);
	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);

	return spi_read_data;
}

void MotorManager_GetSpeed(MotorParameters_t *params)
{
	params->rpm = (MotorSpeed_t) actual_rpm;
	params->direction = target_params.direction;
}

MotorStatus_t MotorManager_GetStatus(void)
{
	return motor_status;
}

MotorDiagnosis_t MotorManager_GetDiagnosis(void)
{
	MotorDiagnosis_t diag;

	if (xSemaphoreTake(xDiagnosticsSemaphore, (TickType_t) 250) == pdTRUE) {
		diag = motor_diag;
		xSemaphoreGive(xDiagnosticsSemaphore);
	}

	return diag;
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

void MotorManager_Init(void)
{
	motor_status = STATUS_STOPPED;
	target_params.direction = DIR_NONE;
	target_params.rpm = 0U;
	requested_rpm_changed = 0U;

	xStatusSemaphore = xSemaphoreCreateMutexStatic(&xStatusMutexBuffer);
	xDiagnosticsSemaphore = xSemaphoreCreateMutexStatic(&xDiagnosticsMutexBuffer);

	xQueue_MotorCommands = xQueueCreateStatic(COMMAND_QUEUE_LENGTH, COMMAND_QUEUE_ITEM_SIZE, ucMotorCommandsQueueStorageArea, &xMotorCommandsStaticQueue);

	xMotorManager_MainTask_Handle = xTaskCreateStatic(MotorManager_MainTask, "MotorManager_Main", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xMainTask_Stack, &xMainTask_Buffer);
	xMotorManager_SpeedControllerTask_Handle = xTaskCreateStatic(MotorManager_SpeedControllerTask, "MotorManager_SpeedController", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xSpeedControllerTask_Stack, &xSpeedControllerTask_Buffer);
	xMotorManager_DiagnosticsTask_Handle = xTaskCreateStatic(MotorManager_DiagnosticsTask, "MotorManager_Diagnostics", MOTOR_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xDiagnosticsTask_Stack, &xDiagnosticsTask_Buffer);
}

void MotorManager_DiagnosticsTask(void *pvParameters)
{
	uint8_t driver_dia_reg;
	uint8_t dia_code;

	while (1U) {
		driver_dia_reg = get_driver_diag();
		dia_code = driver_dia_reg & 0xFU;

		if (xSemaphoreTake(xDiagnosticsSemaphore, (TickType_t) 250) == pdTRUE) {
			motor_diag.OvertemperatureShutdown = driver_dia_reg & (1U << 6U);
			motor_diag.CurrentLimitation = driver_dia_reg & (1U << 4U);

			if (0xCU != dia_code && 0x3U != dia_code && 0xFU != dia_code) {
				motor_diag.ShortCircuitCode = dia_code;
				motor_diag.OpenLoad = 0U;
				motor_diag.Undervoltage = 0U;
			} else {
				motor_diag.ShortCircuitCode = 0U;
				motor_diag.OpenLoad = dia_code == 0xCU;
				motor_diag.Undervoltage = dia_code == 0x3U;
			}

			xSemaphoreGive(xDiagnosticsSemaphore);
		}

		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}

void MotorManager_SpeedControllerTask(void *pvParameters)
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

void MotorManager_MainTask(void *pvParameters)
{
	MotorCommand_t command;

	set_motor_speed(0U);
	set_motor_direction(DIR_NONE);

	while (1U) {
		if (xQueueReceive(xQueue_MotorCommands, &command, (TickType_t) 2000)) {
			if (pdFALSE == verify_command(&command)) {
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
