#include <DAVE.h>

#include "tasks.h"
#include "MotorManager.h"

#define MOTOR_MANAGER_STACK_DEPTH	300U

#define COMMAND_QUEUE_LENGTH		8U
#define COMMAND_QUEUE_ITEM_SIZE		sizeof (MotorCommand_t)

#define MIN_RPM						0x8U
#define PWM_FREQUENCY				20000UL // 20 kHz

typedef enum {
	COMMAND_START, COMMAND_STOP, COMMAND_NOTIFY_ERROR
} MotorCommandType_t;

typedef struct {
	MotorCommandType_t type;
	MotorParameters_t parameters;
	uint16_t revolutions;
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

uint16_t actual_rpm = 0U;
uint16_t temperature = 0U;

/* Make sure to take the xStatusSemaphore before calling this function */
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

	target_params.direction = direction;
}

/* Make sure to take the xStatusSemaphore before calling this function */
void set_motor_speed(MotorSpeed_t speed)
{
	uint32_t duty_cycle = speed * 100L;

	PWM_Stop(&PWM_Motor);

	target_params.duty_cycle = speed;

	if (duty_cycle > 0L) {
		PWM_SetFreqAndDutyCycle(&PWM_Motor, (uint32_t) PWM_FREQUENCY, duty_cycle);
		PWM_Start(&PWM_Motor);
		DIGITAL_IO_SetOutputLow(&DIGITAL_IO_MotorDisable);
	} else {
		DIGITAL_IO_SetOutputHigh(&DIGITAL_IO_MotorDisable);
	}
}

void set_motor_auto_stop(uint16_t revolutions)
{
	COUNTER_Stop(&COUNTER_AutoStop);

	if (revolutions > 0U) {
		COUNTER_ResetCounter(&COUNTER_AutoStop);
		COUNTER_SetCountMatch(&COUNTER_AutoStop, revolutions);
		COUNTER_Start(&COUNTER_AutoStop);
	}
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

void Adc_Measurement_Handler()
{
	DIGITAL_IO_ToggleOutput(&DIGITAL_IO_StatusLED);
	XMC_VADC_RESULT_SIZE_t result = ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_Channel_A);
	uint32_t mv = (48000U / 4096U) * result;

	temperature = mv / 100U;
    return;
}

BaseType_t MotorManager_GetStatus(MotorStatus_t *status)
{
	if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 500) == pdTRUE) {
		*status = motor_status;

		xSemaphoreGive(xStatusSemaphore);
		return pdTRUE;
	}

	return pdFALSE;
}

BaseType_t MotorManager_GetDiagnosis(MotorDiagnosis_t *diagnosis)
{
	if (xSemaphoreTake(xDiagnosticsSemaphore, (TickType_t) 500) == pdTRUE) {
		*diagnosis = motor_diag;

		xSemaphoreGive(xDiagnosticsSemaphore);
		return pdTRUE;
	}

	return pdFALSE;
}

BaseType_t MotorManager_GetRequestedSpeed(MotorParameters_t *params)
{
	if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 50) == pdTRUE) {
		params->duty_cycle = target_params.duty_cycle;
		params->direction = target_params.direction;

		xSemaphoreGive(xStatusSemaphore);
		return pdTRUE;
	}

	return pdFALSE;
}

BaseType_t MotorManager_GetTemperature(uint16_t *temp)
{
	*temp = temperature;

	return pdTRUE;
}

BaseType_t MotorManager_GetRPM(uint16_t *rpm)
{
	if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 500) == pdTRUE) {
		*rpm = actual_rpm;

		xSemaphoreGive(xStatusSemaphore);
		return pdTRUE;
	}

	return pdFALSE;
}

BaseType_t MotorManager_SetSpeed(MotorSpeed_t speed, MotorDirection_t direction, uint16_t revolutions)
{
	MotorCommand_t command;

	command.type = COMMAND_START;
	command.parameters.duty_cycle = speed;
	command.parameters.direction = direction;
	command.revolutions = revolutions;

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

BaseType_t MotorManager_Stop(void)
{
	MotorCommand_t command;

	command.type = COMMAND_STOP;
	command.parameters.duty_cycle = 0U;
	command.parameters.direction = 0U;
	command.revolutions = 0U;

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

BaseType_t MotorManager_NotifyError(void)
{
	MotorCommand_t command;

	command.type = COMMAND_NOTIFY_ERROR;
	command.parameters.duty_cycle = 0U;
	command.parameters.direction = 0U;
	command.revolutions = 0U;

	return xQueueSend(xQueue_MotorCommands, &command, (TickType_t) 2000);
}

void MotorManager_AutoStopHandler(void)
{
	MotorManager_Stop();
	COUNTER_Stop(&COUNTER_AutoStop);
}

void MotorManager_Init(void)
{
	motor_status = STATUS_STOPPED;
	target_params.direction = DIR_NONE;
	target_params.duty_cycle = 0U;

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
	BaseType_t open_load;
	BaseType_t driver_errors = 0U;
	BaseType_t mechanical_errors = 0U;

	while (1U) {
		open_load = 0L;
		driver_dia_reg = get_driver_diag();
		dia_code = driver_dia_reg & 0xFU;

		if (xSemaphoreTake(xDiagnosticsSemaphore, (TickType_t) 250) == pdTRUE) {
			motor_diag.OvertemperatureShutdown = (uint8_t) (0U == (driver_dia_reg & 20U));
			motor_diag.CurrentLimitation = (uint8_t) (0U == (driver_dia_reg & 8U));

			if (0U != motor_diag.OvertemperatureShutdown || 0U != motor_diag.CurrentLimitation) {
				driver_errors += 1L;
			}

			if (0xCU != dia_code && 0x3U != dia_code && 0xFU != dia_code) {
				motor_diag.ShortCircuitCode = dia_code;
				motor_diag.OpenLoad = 0U;
				motor_diag.Undervoltage = 0U;

				if (0U != motor_diag.ShortCircuitCode) {
					driver_errors += 1L;
				}
			} else {
				motor_diag.ShortCircuitCode = 0U;
				motor_diag.OpenLoad = dia_code == 0xCU;
				motor_diag.Undervoltage = dia_code == 0x3U;

				if (0U != motor_diag.Undervoltage) {
					driver_errors += 1L;
				}

				open_load = (BaseType_t) motor_diag.OpenLoad;
			}

			xSemaphoreGive(xDiagnosticsSemaphore);
		}

		if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 100) == pdTRUE) {
			if (STATUS_RUNNING == motor_status) {
				if (MIN_RPM > actual_rpm) {
					//mechanical_errors += 1L;
				} else {
					mechanical_errors = 0L;
				}
			} else {
				if (0L != open_load) {
					driver_errors += 1L;
				}

				mechanical_errors = 0L;
			}

			xSemaphoreGive(xStatusSemaphore);
		}

		if (3L <= driver_errors || 3L <= mechanical_errors) {
			driver_errors = 0L;
			mechanical_errors = 0L;

			MotorManager_NotifyError();
		}

		vTaskDelay(333 / portTICK_PERIOD_MS);
	}
}

void MotorManager_SpeedControllerTask(void *pvParameters)
{
	uint16_t rpm;

	while (1U) {
		rpm = 3U * COUNTER_GetCurrentCount(&COUNTER_WheelRevolution);
		COUNTER_ResetCounter(&COUNTER_WheelRevolution);

		if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 50) == pdTRUE) {
			actual_rpm = rpm;
			xSemaphoreGive(xStatusSemaphore);
		}

		vTaskDelay(333 / portTICK_PERIOD_MS);
	}
}

void MotorManager_MainTask(void *pvParameters)
{
	MotorCommand_t command;

	set_motor_speed(0U);
	set_motor_direction(DIR_NONE);

	while (1U) {
		if (xQueueReceive(xQueue_MotorCommands, &command, (TickType_t) 2000)) {
			if (xSemaphoreTake(xStatusSemaphore, (TickType_t) 2000) == pdTRUE) {
				switch (command.type) {
				case COMMAND_NOTIFY_ERROR:
					motor_status = STATUS_ERROR;

					set_motor_auto_stop(0U);
					set_motor_speed(0U);
					set_motor_direction(DIR_NONE);

					COUNTER_Stop(&COUNTER_WheelRevolution);

					break;
				case COMMAND_START:
					motor_status = STATUS_RUNNING;

					COUNTER_Start(&COUNTER_WheelRevolution);

					set_motor_auto_stop(command.revolutions);
					set_motor_direction(command.parameters.direction);
					set_motor_speed(command.parameters.duty_cycle);

					break;
				case COMMAND_STOP:
				default:
					motor_status = STATUS_STOPPED;

					set_motor_auto_stop(0U);
					set_motor_speed(0U);
					set_motor_direction(DIR_NONE);

					COUNTER_Stop(&COUNTER_WheelRevolution);

					break;
				}

				xSemaphoreGive(xStatusSemaphore);
			}
		}
	}
}
