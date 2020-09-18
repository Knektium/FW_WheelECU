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

// Message structs
typedef struct {
	uint16_t speed;
	uint8_t direction;
} Message_Speed_t;

extern TaskHandle_t xMessageManagerHandle;

void MessageManager_Init(void);
void MessageManager_Main(void *pvParameters);

void MessageManager_PushMessage(Message_t *message);

#endif /* TASKS_MESSAGEMANAGER_H_ */
