#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <FREERTOS/task.h>

#include "Tasks/tasks.h"
#include "Tasks/ModeManager.h"
#include "Tasks/MotorManager.h"
#include "Tasks/MessageManager.h"
#include "Tasks/TemperatureManager.h"
#include "CAN_Config.h"

void Handle_WheelControl_Received(WheelControl_t msg, uint8_t from_node_id, uint8_t to_node_id)
{
	if (NODE_ID != to_node_id) {
		return;
	}

	MotorDirection_t motor_direction;
	uint16_t motor_speed = msg.Speed;

	switch (msg.Direction) {
	case 2U:
		motor_direction = DIR_FORWARD;
		break;
	case 1U:
		motor_direction = DIR_BACKWARD;
		break;
	case 0U:
	default:
		motor_direction = DIR_NONE;
		break;
	}

	if (DIR_NONE == motor_direction) {
		MotorManager_Stop();
	} else {
		MotorManager_SetSpeed(motor_speed, motor_direction, msg.Revolutions);
	}
}

void Task_Main(void *pvParameters)
{
	WheelStatus_t wheel_status;
	MotorStatus_t motor_status;
	MotorParameters_t motor_params;
	MotorParameters_t requested_motor_params;
	MotorDiagnosis_t motor_diag;
	BaseType_t temperature;

	while (1U) {
		MotorManager_GetStatus(&motor_status);
		MotorManager_GetDiagnosis(&motor_diag);
		MotorManager_GetSpeed(&motor_params);
		MotorManager_GetRequestedSpeed(&requested_motor_params);
		TemperatureManager_GetTemperature(&temperature);

		wheel_status.RequestedRevolutionsPerMinute = (uint16_t) requested_motor_params.rpm;
		wheel_status.RevolutionsPerMinute = (uint16_t) motor_params.rpm;
		wheel_status.Direction = motor_params.direction;
		wheel_status.Status = motor_status;

		wheel_status.OvertemperatureShutdown = motor_diag.OvertemperatureShutdown;
		wheel_status.CurrentLimitation = motor_diag.CurrentLimitation;
		wheel_status.ShortCircuitCode = motor_diag.ShortCircuitCode;
		wheel_status.OpenLoad = motor_diag.OpenLoad;
		wheel_status.Undervoltage = motor_diag.Undervoltage;

		wheel_status.Temperature = (uint8_t) temperature;

		MessageManager_Send_WheelStatus(&wheel_status, 0x00);
		DIGITAL_IO_ToggleOutput(&DIGITAL_IO_StatusLED);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

/**

 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */

int main(void)
{
	DAVE_STATUS_t status;

	ModeManager_SetCurrentMode(MODE_STARTUP);
	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	DIGITAL_IO_SetOutputLow(&DIGITAL_IO_StatusLED);

	if (status != DAVE_STATUS_SUCCESS)
	{
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1U)
		{

		}
	}

	/* Create tasks */
	ModeManager_Init();
	MotorManager_Init();
	TemperatureManager_Init();
	MessageManager_Init();
	xTaskCreate(Task_Main, "Task_Main", 40U, NULL, (tskIDLE_PRIORITY + 2), NULL);

	ModeManager_RequestMode(MODE_RUN);

	vTaskStartScheduler();

	/* Placeholder for user application code. The while loop below can be replaced with user application code. */
	while (1U)
	{

	}
}
