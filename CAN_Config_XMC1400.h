#ifndef CAN_NODE_WHEEL_CONF_XMC1400_H
#define CAN_NODE_WHEEL_CONF_XMC1400_H

#include "Dave/Generated/CAN_NODE/can_node.h"

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

/* Message configs */
extern XMC_CAN_MO_t CAN_NODE_Wheel_LMO_WheelControl;
extern const CAN_NODE_LMO_t CAN_NODE_Wheel_LMO_WheelControl_Config;

extern XMC_CAN_MO_t CAN_NODE_Wheel_LMO_WheelStatus;
extern const CAN_NODE_LMO_t CAN_NODE_Wheel_LMO_WheelStatus_Config;


#endif /* End of CAN_NODE_WHEEL_CONF_XMC1400_H */
