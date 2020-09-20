/*
 * MessageManager.c
 *
 *  Created on: 23 Jan 2019
 *      Author: Jack
 */
#include <DAVE.h>

#include "tasks.h"
#include "MessageManager.h"
#include "MotorManager.h"
#include "CAN_Config.h"
#include "CAN_Router.h"

#define MESSAGE_MANAGER_STACK_DEPTH	300U

#define MESSAGE_QUEUE_LENGTH		12U
#define MESSAGE_QUEUE_ITEM_SIZE		sizeof (Message_t)

StaticQueue_t xMessageStaticQueue;
uint8_t ucMessageQueueStorageArea[MESSAGE_QUEUE_LENGTH * MESSAGE_QUEUE_ITEM_SIZE];
QueueHandle_t xQueue_Messages;

TaskHandle_t xMessageManagerHandle = NULL;
StaticTask_t xMessageManagerBuffer;
StackType_t xMessageManagerStack[300U];


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
		MotorManager_SetSpeed(motor_speed, motor_direction);
	}
}

void MessageManager_PushMessage(Message_t *message)
{
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	xQueueSendFromISR(xQueue_Messages, message, &xHigherPriorityTaskWoken);
}

void MessageManager_Init(void)
{
	xQueue_Messages = xQueueCreateStatic(MESSAGE_QUEUE_LENGTH, MESSAGE_QUEUE_ITEM_SIZE, ucMessageQueueStorageArea, &xMessageStaticQueue);
	xMessageManagerHandle = xTaskCreateStatic(MessageManager_Main, "MessageManager", MESSAGE_MANAGER_STACK_DEPTH, NULL, (tskIDLE_PRIORITY + 2), xMessageManagerStack, &xMessageManagerBuffer);
}

void MessageManager_Main(void *pvParameters)
{
	Message_t message;

	while (1U) {
		if (xQueueReceive(xQueue_Messages, &message, (TickType_t) 2000)) {
			// Route it
			CAN_HandleReceivedMessage(message);
		}
	}
}
