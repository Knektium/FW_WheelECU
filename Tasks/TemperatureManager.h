#ifndef TASKS_TEMPERATUREMANAGER_H_
#define TASKS_TEMPERATUREMANAGER_H_

extern TaskHandle_t xTemperatureManager_MainTask_Handle;


// Tasks
void TemperatureManager_MainTask(void *pvParameters);

// Functions
void TemperatureManager_Init(void);
BaseType_t TemperatureManager_GetTemperature(BaseType_t *temperature);

// Interrupt handlers
void TemperatureManager_AdcHandler(void);

#endif /* TASKS_TEMPERATUREMANAGER_H_ */
