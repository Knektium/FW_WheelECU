/*
 * MessageManager.h
 *
 *  Created on: 26 Jan 2019
 *      Author: Jack
 */

#ifndef TASKS_MESSAGEMANAGER_H_
#define TASKS_MESSAGEMANAGER_H_

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint32_t crc;
} Message_t;

// Message ID definitions
#define MESSAGE_SPEED_ID 0x1000000U

// Message structs
typedef struct {
	uint8_t speed;
	uint8_t direction;
} Message_Speed_t;

extern TaskHandle_t xMessageManagerHandle;

void MessageManager_Init(void);
void MessageManager_Main(void *pvParameters);

void MessageManager_PushMessage(Message_t *message);

#endif /* TASKS_MESSAGEMANAGER_H_ */
