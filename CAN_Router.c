#include <DAVE.h>
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


void Send_WheelStatus(WheelStatus_t *s, uint32_t to_node)
{
	uint8_t data[8] = { 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U };

	data[0U] |= ((uint8_t) (s->RevolutionsPerMinute >> 0U) << 0U) & 255U;
	data[1U] |= ((uint8_t) (s->RevolutionsPerMinute >> 8U) << 0U) & 255U;
	
	data[2U] |= ((uint8_t) (s->ErrorCode >> 0U) << 0U) & 255U;
	
	data[3U] |= ((uint8_t) (s->Status >> 0U) << 0U) & 3U;
	
	data[3U] |= ((uint8_t) (s->Direction >> 0U) << 2U) & 12U;
	
	CAN_NODE_MO_UpdateData(CAN_NODE_0.lmobj_ptr[CAN_MESSAGE_WHEELSTATUS_INDEX], (uint8_t *) data);
	CAN_NODE_MO_Transmit(CAN_NODE_0.lmobj_ptr[CAN_MESSAGE_WHEELSTATUS_INDEX]);
}

