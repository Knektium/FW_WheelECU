#ifndef CAN_ROUTER_H
#define CAN_ROUTER_H

#include <stdint.h>

typedef struct {
	uint32_t id;
	uint8_t data[8];
	uint32_t crc;
} Message_t;

/* Message struct typedefs */
typedef struct WheelControl_s {
	uint16_t Speed;
	uint8_t Direction;
	uint16_t Revolutions;
} WheelControl_t;

typedef struct WheelStatus_s {
	uint16_t RevolutionsPerMinute;
	uint8_t OvertemperatureShutdown;
	uint8_t CurrentLimitation;
	uint8_t OpenLoad;
	uint8_t Undervoltage;
	uint8_t ShortCircuitCode;
	uint8_t Status;
	uint8_t Direction;
	uint16_t RequestedRevolutionsPerMinute;
} WheelStatus_t;

void CAN_HandleReceivedMessage(Message_t message);
extern void Handle_WheelControl_Received(WheelControl_t msg, uint8_t from_node_id, uint8_t to_node_id);
void Send_WheelStatus(WheelStatus_t *s, uint32_t to_node);


#endif /* End of CAN_ROUTER_H */
