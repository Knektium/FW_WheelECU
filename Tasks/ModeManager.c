#include <DAVE.h>

//#include "../timers.h"
#include "tasks.h"
#include "ModeManager.h"

#include "MotorManager.h"

StaticQueue_t xModeRequestStaticQueue;
uint8_t ucModeRequestQueueStorageArea[5 * sizeof (ApplicationMode)];
QueueHandle_t xQueue_ModeRequest;

TaskHandle_t xModeManagerHandle = NULL;
StaticTask_t xModeManagerBuffer;
StackType_t xModeManagerStack[300U];

ApplicationMode current_mode;

void ModeManager_Init(void)
{
	xModeManagerHandle = xTaskCreateStatic(ModeManager_Main, "ModeManager", 300U, NULL, (tskIDLE_PRIORITY + 4), xModeManagerStack, &xModeManagerBuffer);
	xQueue_ModeRequest = xQueueCreateStatic(10, sizeof (ApplicationMode), ucModeRequestQueueStorageArea, &xModeRequestStaticQueue);

	current_mode = MODE_STARTUP;
}

void ModeManager_RequestMode(ApplicationMode mode)
{
	xQueueSend(xQueue_ModeRequest, &mode, (TickType_t) 0);
}

static void ModeManager_SuspendAllTasks(void)
{
	vTaskSuspend(xMotorManagerHandle);
}

static void ModeManager_StopAllTimers(void)
{
	//xTimerStop(xTimer_ReadSensorsHandle, 0);
}

void ModeManager_Main(void *pvParameters)
{
	ApplicationMode requested_mode = MODE_STARTUP;

	while(1U)
	{
		if (xQueueReceive(xQueue_ModeRequest, &requested_mode, (TickType_t) 2000)) {
			//vTaskSuspendAll();			/* Suspend scheduler */
			//ModeManager_SuspendAllTasks();	/* Suspend all tasks */
			ModeManager_StopAllTimers();	/* Stop all timers */

			current_mode = requested_mode;

			switch (requested_mode) {
			case MODE_RUN:
				/* Reset queues (a late coming ISR could have pushed something to a queue) */
				/* ... */

				/* Start timers */
				//xTimerStart(xTimer_ReadSensorsHandle, 0);

				/* Start tasks */
				//vTaskResume(xMotorManagerHandle);

				break;

			case MODE_SLEEP:
				/* Disable all interrupts except RTC */
				/* ... */

				/* Disable sensors */
				//DIGITAL_IO_SetOutputLow(&DIGITAL_IO_SensorsEnable);

				/* Enter sleep mode */
				__WFI();

				/* Woken up, do something... */

				break;
			default:

				break;
			}

			/* Resume scheduler */
			//xTaskResumeAll();
		}
	}
}

