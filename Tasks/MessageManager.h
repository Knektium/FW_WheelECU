#ifndef TASKS_MESSAGEMANAGER_H_
#define TASKS_MESSAGEMANAGER_H_

#include "CAN_Router.h"

extern TaskHandle_t xMessageManagerHandle;

void MessageManager_Init(void);
void MessageManager_Main(void *pvParameters);
void MessageManager_PeriodicTask(void *pvParameters);

void MessageManager_PushMessage(Message_t *message);

#endif /* TASKS_MESSAGEMANAGER_H_ */
