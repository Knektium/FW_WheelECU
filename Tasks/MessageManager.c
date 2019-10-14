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

#define MESSAGE_MANAGER_STACK_DEPTH	300U

#define MESSAGE_QUEUE_LENGTH		12U
#define MESSAGE_QUEUE_ITEM_SIZE		sizeof (Message_t)

StaticQueue_t xMessageStaticQueue;
uint8_t ucMessageQueueStorageArea[MESSAGE_QUEUE_LENGTH * MESSAGE_QUEUE_ITEM_SIZE];
QueueHandle_t xQueue_Messages;

TaskHandle_t xMessageManagerHandle = NULL;
StaticTask_t xMessageManagerBuffer;
StackType_t xMessageManagerStack[300U];


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
			if ((message.id & MESSAGE_SPEED_ID) == MESSAGE_SPEED_ID) {
				// Marshall message
				Message_Speed_t speed_message;

				speed_message.speed = message.data[0];
				speed_message.direction = message.data[1];

				// Handle message
				MotorSpeed_t motor_speed;
				MotorDirection_t motor_direction;

				switch (speed_message.speed) {
				case 2U:
					motor_speed = SPEED_HIGH;
					break;
				case 1U:
					motor_speed = SPEED_MEDIUM;
					break;
				case 0U:
				default:
					motor_speed = SPEED_LOW;
					break;
				}

				switch (speed_message.direction) {
				case 2U:
					motor_direction = MOTOR_FORWARD;
					break;
				case 1U:
					motor_direction = MOTOR_BACKWARD;
					break;
				case 0U:
				default:
					motor_direction = MOTOR_STILL;
					break;
				}

				if (MOTOR_STILL == motor_direction) {
					MotorManager_Stop();
				} else {
					MotorManager_SetSpeed(motor_speed, motor_direction);
				}
			}
		}
	}
}
