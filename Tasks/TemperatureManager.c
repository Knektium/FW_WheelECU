#include <DAVE.h>

#include "tasks.h"
#include "TemperatureManager.h"

#define TEMPERATURE_MANAGER_STACK_DEPTH	300U

// Task handles, buffers and stacks
TaskHandle_t xTemperatureManager_MainTask_Handle = NULL;
StaticTask_t xTemperatureManager_MainTask_Buffer;
StackType_t xTemperatureManager_MainTask_Stack[300U];

// Semaphores
SemaphoreHandle_t xTemperatureSemaphore = NULL;
StaticSemaphore_t xTemperatureMutexBuffer;

BaseType_t last_temperature;

void TemperatureManager_AdcHandler(void)
{
	XMC_VADC_RESULT_SIZE_t result;
	uint32_t millivolt;

	result = ADC_MEASUREMENT_GetResult(&ADC_MEASUREMENT_Channel_A);
	millivolt = (48000UL / 4096UL) * result;

	if (xSemaphoreTake(xTemperatureSemaphore, (TickType_t) 2000) == pdTRUE) {
		last_temperature = millivolt / 100UL;

		xSemaphoreGive(xTemperatureSemaphore);
	}

	return;
}

BaseType_t TemperatureManager_GetTemperature(BaseType_t *temperature)
{
	if (xSemaphoreTake(xTemperatureSemaphore, (TickType_t) 500) == pdTRUE) {
		*temperature = last_temperature;

		xSemaphoreGive(xTemperatureSemaphore);
		return pdTRUE;
	}

	return pdFALSE;
}

void TemperatureManager_Init(void)
{
	last_temperature = 0;

	xTemperatureSemaphore = xSemaphoreCreateMutexStatic(&xTemperatureMutexBuffer);

	xTemperatureManager_MainTask_Handle = xTaskCreateStatic(TemperatureManager_MainTask, "TemperatureManager_Main", TEMPERATURE_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 3), xTemperatureManager_MainTask_Stack, &xTemperatureManager_MainTask_Buffer);
}

void TemperatureManager_MainTask(void *pvParameters)
{
	while (1U) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		TemperatureManager_AdcHandler();
	}
}
