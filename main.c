/*
 * main.c
 *
 *  Created on: 2018 Nov 15 19:38:19
 *  Author: Jack
 */




#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <FREERTOS/task.h>

#include "Tasks/tasks.h"
#include "Tasks/ModeManager.h"
#include "Tasks/MotorManager.h"
#include "Tasks/MessageManager.h"

// SPI command set
#define RD_DIA	0x00U
#define RES_DIA	0x80U
#define RD_REV	0x20U
#define RD_CTRL	0x60U
#define WR_CTRL_RD_DIA	0xD0U
#define WR_CTRL	0xE0U

void Time_Handler(void)
{
	uint16_t data[8];

	uint8_t spi_read_data;
	uint8_t spi_write_data;

	spi_read_data = 0xFFU;
	spi_write_data = RES_DIA;
	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);

	spi_write_data = RD_DIA;
	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);

	// Dummy message to get answer to first message
	SPI_MASTER_Transfer(&SPI_MASTER_0, &spi_write_data, &spi_read_data, 1);

	data[0] = MotorManager_GetRPM();
	data[1] = (uint16_t) 89U;
	data[2] = (uint16_t) 0U;
	data[3] = (uint16_t) spi_read_data;
	CAN_NODE_MO_UpdateData(CAN_NODE_0.lmobj_ptr[1], (uint8_t *) data);
	CAN_NODE_MO_Transmit(CAN_NODE_0.lmobj_ptr[1]);
}

void EventHandler_CanNode_0()
{
	// Check transmit pending status in LMO_01
	uint32_t status = 0x00U;

	// Check for Node error
	status = CAN_NODE_GetStatus(&CAN_NODE_0);
	if (status & XMC_CAN_NODE_STATUS_ALERT_WARNING)
	{
		// Clear the flag
		CAN_NODE_DisableEvent(&CAN_NODE_0, XMC_CAN_NODE_EVENT_ALERT);
	}

	status = CAN_NODE_MO_GetStatus((void *) CAN_NODE_0.lmobj_ptr[1]);
	if (status & XMC_CAN_MO_STATUS_TX_PENDING)
	{
		// Clear the flag
		CAN_NODE_MO_ClearStatus((void *)CAN_NODE_0.lmobj_ptr[1], XMC_CAN_MO_RESET_STATUS_TX_PENDING);
	}

	// Check receive pending status in LMO_02
	status = CAN_NODE_MO_GetStatus((void *)CAN_NODE_0.lmobj_ptr[0]);
	if (status & XMC_CAN_MO_STATUS_RX_PENDING) //XMC_CAN_MO_STATUS_NEW_DATA
	{
		// Clear the flag
		CAN_NODE_MO_ClearStatus((void *) CAN_NODE_0.lmobj_ptr[0], XMC_CAN_MO_RESET_STATUS_RX_PENDING);

		// Read the received Message object and stores the received data in the MO structure.
		CAN_NODE_MO_Receive((void *) CAN_NODE_0.lmobj_ptr[0]);

		if (ModeManager_GetCurrentMode() == MODE_RUN) {
			Message_t message;
			uint8_t *can_data = CAN_NODE_0.lmobj_ptr[0]->mo_ptr->can_data_byte;

			message.id = CAN_NODE_0.lmobj_ptr[0]->mo_ptr->can_identifier;

			message.data[0] = can_data[0];
			message.data[1] = can_data[1];
			message.data[2] = can_data[2];
			message.data[3] = can_data[3];
			message.data[4] = can_data[4];
			message.data[5] = can_data[5];
			message.data[6] = can_data[6];
			message.data[7] = can_data[7];

			MessageManager_PushMessage(&message);
		}
	}
}

/**

 * @brief main() - Application entry point
 *
 * <b>Details of function</b><br>
 * This routine is the application entry point. It is invoked by the device startup code. It is responsible for
 * invoking the APP initialization dispatcher routine - DAVE_Init() and hosting the place-holder for user application
 * code.
 */

int main(void)
{
	DAVE_STATUS_t status;

	ModeManager_SetCurrentMode(MODE_STARTUP);
	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	if (status != DAVE_STATUS_SUCCESS)
	{
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1U)
		{

		}
	}

	/* Create tasks */
	ModeManager_Init();
	MotorManager_Init();
	MessageManager_Init();

	ModeManager_RequestMode(MODE_RUN);

	vTaskStartScheduler();

	/* Placeholder for user application code. The while loop below can be replaced with user application code. */
	while (1U)
	{

	}
}
