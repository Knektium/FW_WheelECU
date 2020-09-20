#include "CAN_Config.h"
#include "CAN_Router.h"

void CAN_HandleReceivedMessage(Message_t message)
{
	uint8_t from_node_id = (message.id >> 4U) & 0xFFU;
	uint8_t to_node_id = (message.id >> 12U) & 0xFFU;

	if ((message.id & CAN_MESSAGE_WHEELCONTROL_MASK) == CAN_MESSAGE_WHEELCONTROL_MASK) {
		WheelControl_t s;

		// Parse WheelControl to struct

		s.Speed = 0U;
		s.Speed += ((message.data[0U]) & 255U);
		s.Speed += ((message.data[1U]) & 255U) << 8U;

		s.Direction = 0U;
		s.Direction += ((message.data[2U]) & 3U);

		Handle_WheelControl_Received(s, from_node_id, to_node_id);
	}
	
}
