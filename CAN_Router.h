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
} WheelControl_t;

typedef struct WheelStatus_s {
	uint16_t RevolutionsPerMinute;
	uint16_t ErrorCode;
} WheelStatus_t;

void CAN_HandleReceivedMessage(Message_t message);
void Handle_WheelControl_Received(WheelControl_t msg, uint8_t from_node_id, uint8_t to_node_id);


#endif /* End of CAN_ROUTER_H */