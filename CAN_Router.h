#ifndef CAN_ROUTER_H
#define CAN_ROUTER_H

#include <stdint.h>

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint32_t crc;
} Message_t;

/* Message struct typedefs */
typedef struct FanControl_s {
	uint8_t DutyCycle;
	uint8_t Direction;
	uint16_t Revolutions;
} FanControl_t;

typedef struct FanStatus_s {
	uint16_t RevolutionsPerMinute;
	uint8_t OvertemperatureShutdown;
	uint8_t CurrentLimitation;
	uint8_t OpenLoad;
	uint8_t Undervoltage;
	uint8_t ShortCircuitCode;
	uint8_t Status;
	uint8_t Direction;
	uint8_t DutyCycle;
	uint8_t Temperature;
} FanStatus_t;

void CAN_HandleReceivedMessage(Message_t message);
extern void Handle_FanControl_Received(FanControl_t msg, uint8_t from_node_id, uint8_t to_node_id);
void Send_FanStatus(FanStatus_t *s, uint32_t to_node);


#endif /* End of CAN_ROUTER_H */
