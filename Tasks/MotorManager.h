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

extern TaskHandle_t xMotorManagerHandle;
extern TaskHandle_t xSpeedControllerHandle;

void MotorManager_Init(void);
void MotorManager_Main(void *pvParameters);
void MotorManager_SpeedController(void *pvParameters);
void MotorManager_DiagnosticsTask(void *pvParameters);

BaseType_t MotorManager_SetSpeed(MotorSpeed_t rpm, MotorDirection_t direction);
BaseType_t MotorManager_Stop(void);
void MotorManager_GetSpeed(MotorParameters_t *params);
MotorStatus_t MotorManager_GetStatus(void);
MotorDiagnosis_t MotorManager_GetDiagnosis(void);

#endif /* TASKS_MOTORMANAGER_H_ */
