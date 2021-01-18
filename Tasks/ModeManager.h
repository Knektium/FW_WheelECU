#ifndef TASKS_MODEMANAGER_H_
#define TASKS_MODEMANAGER_H_

typedef enum {
	MODE_STARTUP, MODE_SLEEP, MODE_RUN
} ApplicationMode;

extern TaskHandle_t xModeManagerHandle;

void ModeManager_Init(void);
void ModeManager_Main(void *pvParameters);
void ModeManager_RequestMode(ApplicationMode mode);

ApplicationMode ModeManager_GetCurrentMode(void);
void ModeManager_SetCurrentMode(ApplicationMode);

#endif /* TASKS_MODEMANAGER_H_ */
