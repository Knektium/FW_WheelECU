/*
 * ModeManager.h
 *
 *  Created on: 5 Sep 2018
 *      Author: Jack
 */

#ifndef TASKS_MODEMANAGER_H_
#define TASKS_MODEMANAGER_H_

typedef enum {
	MODE_STARTUP, MODE_SLEEP, MODE_RUN
} ApplicationMode;

extern TaskHandle_t xModeManagerHandle;

void ModeManager_Init(void);
void ModeManager_Main(void *pvParameters);
void ModeManager_RequestMode(ApplicationMode mode);

#endif /* TASKS_MODEMANAGER_H_ */