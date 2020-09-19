#ifndef CAN_NODE_WHEEL_CONF_H
#define CAN_NODE_WHEEL_CONF_H

#include "Dave/Generated/CAN_NODE/can_node.h"

#define NODE_ID 0x11U

/* Message masks */
#define CAN_MESSAGE_WHEELCONTROL_MASK 0x100000U
#define CAN_MESSAGE_WHEELSTATUS_MASK 0x200000U


/* Message struct typedefs */
typedef struct WheelControl_s {
	uint16_t Speed;
	uint8_t Direction;
} WheelControl_t;
typedef struct WheelStatus_s {
	uint16_t RevolutionsPerMinute;
	uint16_t ErrorCode;
} WheelStatus_t;


/* Message configs */

extern XMC_CAN_MO_t CAN_NODE_Wheel_LMO_WheelControl;
extern const CAN_NODE_LMO_t CAN_NODE_Wheel_LMO_WheelControl_Config;

extern XMC_CAN_MO_t CAN_NODE_Wheel_LMO_WheelStatus;
extern const CAN_NODE_LMO_t CAN_NODE_Wheel_LMO_WheelStatus_Config;


#define CAN_MESSAGE_CONFIGS { \
	(CAN_NODE_LMO_t *) &CAN_NODE_Wheel_LMO_WheelControl_Config, \
	(CAN_NODE_LMO_t *) &CAN_NODE_Wheel_LMO_WheelStatus_Config, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL, \
	NULL \
}

#define NUMBER_OF_CAN_MESSAGE_CONFIGS 2U

#endif /* End of CAN_NODE_WHEEL_CONF_H */
