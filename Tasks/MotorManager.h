#ifndef TASKS_MOTORMANAGER_H_
#define TASKS_MOTORMANAGER_H_

typedef uint16_t MotorSpeed_t;

typedef enum {
	DIR_NONE = 0U,
	DIR_FORWARD = 1U,
	DIR_BACKWARD = 2U
} MotorDirection_t;

typedef struct {
	MotorSpeed_t rpm;
	MotorDirection_t direction;
} MotorParameters_t;

typedef struct {
	uint8_t OvertemperatureShutdown;
	uint8_t CurrentLimitation;
	uint8_t OpenLoad;
	uint8_t Undervoltage;
	uint8_t ShortCircuitCode;
} MotorDiagnosis_t;

typedef enum {
	STATUS_STOPPED = 0U,
	STATUS_RUNNING = 1U
} MotorStatus_t;

extern TaskHandle_t xMotorManager_MainTask_Handle;
extern TaskHandle_t xMotorManager_SpeedControllerTask_Handle;
extern TaskHandle_t xMotorManager_DiagnosticsTask_Handle;

// Tasks
void MotorManager_MainTask(void *pvParameters);
void MotorManager_SpeedControllerTask(void *pvParameters);
void MotorManager_DiagnosticsTask(void *pvParameters);

// Functions
void MotorManager_Init(void);
void MotorManager_GetStatus(MotorStatus_t *status);
void MotorManager_GetDiagnosis(MotorDiagnosis_t *diagnosis);
void MotorManager_GetSpeed(MotorParameters_t *params);
BaseType_t MotorManager_SetSpeed(MotorSpeed_t rpm, MotorDirection_t direction);
BaseType_t MotorManager_Stop(void);

#endif /* TASKS_MOTORMANAGER_H_ */
