#ifndef TASKS_MOTORMANAGER_H_
#define TASKS_MOTORMANAGER_H_

typedef uint8_t MotorSpeed_t;

typedef enum {
	DIR_NONE = 0U,
	DIR_FORWARD = 1U,
	DIR_BACKWARD = 2U
} MotorDirection_t;

typedef struct {
	MotorSpeed_t duty_cycle;
	MotorDirection_t direction;
} MotorParameters_t;

typedef struct {
	uint8_t OvertemperatureShutdown;
	uint8_t CurrentLimitation;
	uint8_t OpenLoad;
	uint8_t Undervoltage;
	uint8_t ShortCircuitCode;
	uint8_t NoRotation;
} MotorDiagnosis_t;

typedef enum {
	STATUS_STOPPED = 0U,
	STATUS_RUNNING = 1U,
	STATUS_ERROR = 2U
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
BaseType_t MotorManager_GetStatus(MotorStatus_t *status);
BaseType_t MotorManager_GetDiagnosis(MotorDiagnosis_t *diagnosis);
BaseType_t MotorManager_GetRequestedParameters(MotorParameters_t *params);
BaseType_t MotorManager_GetTemperature(uint16_t *temp);
BaseType_t MotorManager_GetRPM(uint16_t *rpm);
BaseType_t MotorManager_SetParameters(MotorSpeed_t duty_cycle, MotorDirection_t direction, uint16_t revolutions);
BaseType_t MotorManager_Stop(void);
BaseType_t MotorManager_NotifyError(void);

// Interrupt handlers
void MotorManager_AutoStopHandler(void);

#endif /* TASKS_MOTORMANAGER_H_ */
