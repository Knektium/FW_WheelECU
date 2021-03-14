#include "Dave/Generated/CAN_NODE/can_node.h"


XMC_CAN_MO_t CAN_NODE_Fan_LMO_FanControl = {
	.can_mo_type      = XMC_CAN_MO_TYPE_RECMSGOBJ,
	.can_identifier   = 0x30000bU,
	.can_id_mode      = XMC_CAN_FRAME_TYPE_EXTENDED_29BITS,
	.can_priority     = XMC_CAN_ARBITRATION_MODE_IDE_DIR_BASED_PRIO_2,
	.can_id_mask      = 0x300000U,
	.can_ide_mask     = 1U,
	.can_mo_ptr       = (CAN_MO_TypeDef*) CAN_MO1,
	.can_data_length  = 8U,
	.can_data[1]      = 0x00000000U,
	.can_data[0]      = 0x00000000U
};

const CAN_NODE_LMO_t CAN_NODE_Fan_LMO_FanControl_Config = {
	.mo_ptr  = (XMC_CAN_MO_t*) &CAN_NODE_Fan_LMO_FanControl,
	.number  = 1U,
	.tx_sr   = 0U,
	.rx_sr   = 0U,
	.tx_event_enable = false,
	.rx_event_enable = true
};

XMC_CAN_MO_t CAN_NODE_Fan_LMO_FanStatus = {
	.can_mo_type      = XMC_CAN_MO_TYPE_TRANSMSGOBJ,
	.can_identifier   = 0x400219U,
	.can_id_mode      = XMC_CAN_FRAME_TYPE_EXTENDED_29BITS,
	.can_priority     = XMC_CAN_ARBITRATION_MODE_IDE_DIR_BASED_PRIO_2,
	.can_id_mask      = 0x400000U,
	.can_ide_mask     = 1U,
	.can_mo_ptr       = (CAN_MO_TypeDef*) CAN_MO2,
	.can_data_length  = 8U,
	.can_data[1]      = 0x00000000U,
	.can_data[0]      = 0x00000000U
};

const CAN_NODE_LMO_t CAN_NODE_Fan_LMO_FanStatus_Config = {
	.mo_ptr  = (XMC_CAN_MO_t*) &CAN_NODE_Fan_LMO_FanStatus,
	.number  = 2U,
	.tx_sr   = 0U,
	.rx_sr   = 0U,
	.tx_event_enable = true,
	.rx_event_enable = false
};

