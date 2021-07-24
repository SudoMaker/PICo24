/*
    This file is part of PotatoPi PICo24 SDK.

    Copyright (C) 2021 ReimuNotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* This file contains code originally written by Microchip Technology Inc., which was licensed under the Apache License. */


#include <string.h>
#include "I2C.h"

#include <PICo24/UnixAPI/mini_stdio.h>
#include <PICo24/UnixAPI/mini_unistd.h>

#ifdef PICo24_Enable_Peripheral_I2C


#define I2C_TRANSMIT_REG                       (*hi2c->TRN)			// Defines the transmit register used to send data.
#define I2C_RECEIVE_REG                        (*hi2c->RCV)			// Defines the receive register used to receive data.
#define I2C_WRITE_COLLISION_STATUS_BIT         hi2c->STAT->IWCOL		// Defines the write collision status bit.
#define I2C_ACKNOWLEDGE_STATUS_BIT             hi2c->STAT->ACKSTAT		// I2C ACK status bit.
#define I2C_START_CONDITION_ENABLE_BIT         hi2c->CON->SEN			// I2C START control bit.
#define I2C_REPEAT_START_CONDITION_ENABLE_BIT  hi2c->CON->RSEN			// I2C Repeated START control bit.
#define I2C_RECEIVE_ENABLE_BIT                 hi2c->CON->RCEN			// I2C Receive enable control bit.
#define I2C_STOP_CONDITION_ENABLE_BIT          hi2c->CON->PEN			// I2C STOP control bit.
#define I2C_ACKNOWLEDGE_ENABLE_BIT             hi2c->CON->ACKEN 		// I2C ACK start control bit.
#define I2C_ACKNOWLEDGE_DATA_BIT               hi2c->CON->ACKDT			// I2C ACK data control bit.



static void I2C_Master_FunctionComplete(I2C_HandleTypeDef *hi2c);
static void I2C_Master_Stop(I2C_HandleTypeDef *hi2c, I2C_MESSAGE_STATUS completion_code);


void I2C_Master_Initialize(I2C_HandleTypeDef *hi2c, uint8_t i2c_mode, uint8_t queue_len, uint16_t speed) {
	*hi2c->BRG = speed;

	memset(&hi2c->master, 0, sizeof(I2C_MASTER_OBJECT));
	hi2c->master.queue_len = queue_len;
	hi2c->master.i2c_tr_queue = malloc(queue_len * sizeof(I2C_TR_QUEUE_ENTRY));
	hi2c->master.pTrHead = hi2c->master.i2c_tr_queue;
	hi2c->master.pTrTail = hi2c->master.i2c_tr_queue;
	hi2c->master.trStatus.s.empty = true;
	hi2c->master.trStatus.s.full = false;
	hi2c->master.i2cErrors = 0;


	*(uint16_t *)hi2c->CON = 0x0000;

	if (i2c_mode & I2C_10BIT)
		hi2c->CON->A10M = 1;

	// BCL disabled; P disabled; S disabled; I2COV disabled; IWCOL disabled;
	*(uint16_t *)hi2c->STAT = 0x00;

	// clear the master interrupt flag
	*hi2c->IFS &= ~(1U << hi2c->IFSOffset_M);

	// enable the master interrupt
	*hi2c->IEC |= 1U << hi2c->IECOffset_M;

	hi2c->CON->I2CEN = 1;
}


uint8_t I2C_Master_GetErrorCount(I2C_HandleTypeDef *hi2c) {
	return hi2c->master.i2cErrors;
}

void I2C_Master_ProcessInterrupt(I2C_HandleTypeDef *hi2c) {
	*hi2c->IFS &= ~(1U << hi2c->IFSOffset_M);

	// Check first if there was a collision.
	// If we have a Write Collision, reset and go to idle state */
	if (hi2c->STAT->IWCOL) {
		// clear the Write colision
		hi2c->STAT->IWCOL = 0;
		hi2c->master.i2c1_state = S_MASTER_IDLE;
		*(hi2c->master.p_i2c1_current->pTrFlag) = I2C_MESSAGE_FAIL;

		// reset the buffer pointer
		hi2c->master.p_i2c1_current = NULL;

		return;
	}

	/* Handle the correct i2c state */
	switch (hi2c->master.i2c1_state) {
		case S_MASTER_IDLE:    /* In reset state, waiting for data to send */
			if (hi2c->master.trStatus.s.empty != true) {
				// grab the item pointed by the head
				hi2c->master.p_i2c1_current     = hi2c->master.pTrHead;
				hi2c->master.i2c1_trb_count     = hi2c->master.pTrHead->count;
				hi2c->master.p_i2c1_trb_current = hi2c->master.pTrHead->ptrb_list;

				hi2c->master.pTrHead++;

				// check if the end of the array is reached
				if(hi2c->master.pTrHead == (hi2c->master.i2c_tr_queue + hi2c->master.queue_len)) {
					// adjust to restart at the beginning of the array
					hi2c->master.pTrHead = hi2c->master.i2c_tr_queue;
				}

				// since we moved one item to be processed, we know
				// it is not full, so set the full status to false
				hi2c->master.trStatus.s.full = false;

				// check if the queue is empty
				if(hi2c->master.pTrHead == hi2c->master.pTrTail) {
					// it is empty so set the empty status to true
					hi2c->master.trStatus.s.empty = true;
				}

				// send the start condition
				I2C_START_CONDITION_ENABLE_BIT = 1;

				// start the i2c request
				hi2c->master.i2c1_state = S_MASTER_SEND_ADDR;
			}

			break;

		case S_MASTER_RESTART:

			/* check for pending i2c Request */

			// ... trigger a REPEATED START
			I2C_REPEAT_START_CONDITION_ENABLE_BIT = 1;

			// start the i2c request
			hi2c->master.i2c1_state = S_MASTER_SEND_ADDR;

			break;

		case S_MASTER_SEND_ADDR_10BIT_LSB:

			if (I2C_ACKNOWLEDGE_STATUS_BIT) {
				hi2c->master.i2cErrors++;
				I2C_Master_Stop(hi2c, I2C_MESSAGE_ADDRESS_NO_ACK);
			} else {
				// Remove bit 0 as R/W is never sent here
				I2C_TRANSMIT_REG = (hi2c->master.i2c_address >> 1) & 0x00FF;

				// determine the next state, check R/W
				if (hi2c->master.i2c_address & 0x01) {
					// if this is a read we must repeat start
					// the bus to perform a read
					hi2c->master.i2c1_state = S_MASTER_10BIT_RESTART;
				} else {
					// this is a write continue writing data
					hi2c->master.i2c1_state = S_MASTER_SEND_DATA;
				}
			}

			break;

		case S_MASTER_10BIT_RESTART:

			if (I2C_ACKNOWLEDGE_STATUS_BIT) {
				hi2c->master.i2cErrors++;
				I2C_Master_Stop(hi2c, I2C_MESSAGE_ADDRESS_NO_ACK);
			} else {
				// ACK Status is good
				// restart the bus
				I2C_REPEAT_START_CONDITION_ENABLE_BIT = 1;

				// fudge the address so S_MASTER_SEND_ADDR works correctly
				// we only do this on a 10-bit address resend
				hi2c->master.i2c_address = 0x00F0 | ((hi2c->master.i2c_address >> 8) & 0x0006);

				// set the R/W flag
				hi2c->master.i2c_address |= 0x0001;

				// set the address restart flag so we do not change the address
				hi2c->master.i2c_10bit_address_restart = 1;

				// Resend the address as a read
				hi2c->master.i2c1_state = S_MASTER_SEND_ADDR;
			}

			break;

		case S_MASTER_SEND_ADDR:

			/* Start has been sent, send the address byte */

			/* Note:
			    On a 10-bit address resend (done only during a 10-bit
			    device read), the original i2c_address was modified in
			    S_MASTER_10BIT_RESTART state. So the check if this is
			    a 10-bit address will fail and a normal 7-bit address
			    is sent with the R/W bit set to read. The flag
			    i2c_10bit_address_restart prevents the  address to
			    be re-written.
			 */
			if (hi2c->master.i2c_10bit_address_restart != 1) {
				// extract the information for this message
				hi2c->master.i2c_address    = hi2c->master.p_i2c1_trb_current->address;
				hi2c->master.pi2c_buf_ptr   = hi2c->master.p_i2c1_trb_current->pbuffer;
				hi2c->master.i2c_bytes_left = hi2c->master.p_i2c1_trb_current->length;
			} else {
				// reset the flag so the next access is ok
				hi2c->master.i2c_10bit_address_restart = 0;
			}

			// check for 10-bit address
			if (hi2c->master.i2c_address > 0x00FF) {
				// we have a 10 bit address
				// send bits<9:8>
				// mask bit 0 as this is always a write
				I2C_TRANSMIT_REG = 0xF0 | ((hi2c->master.i2c_address >> 8) & 0x0006);
				hi2c->master.i2c1_state = S_MASTER_SEND_ADDR_10BIT_LSB;
			} else {
				// Transmit the address
				I2C_TRANSMIT_REG = hi2c->master.i2c_address;
				if (hi2c->master.i2c_address & 0x01) {
					// Next state is to wait for address to be acked
					hi2c->master.i2c1_state = S_MASTER_ACK_ADDR;
				} else {
					// Next state is transmit
					hi2c->master.i2c1_state = S_MASTER_SEND_DATA;
				}
			}
			break;

		case S_MASTER_SEND_DATA:

			// Make sure the previous byte was acknowledged
			if (I2C_ACKNOWLEDGE_STATUS_BIT) {
				// Transmission was not acknowledged
				hi2c->master.i2cErrors++;

				// Reset the Ack flag
				I2C_ACKNOWLEDGE_STATUS_BIT = 0;

				// Send a stop flag and go back to idle
				I2C_Master_Stop(hi2c, I2C_DATA_NO_ACK);

			} else {
				// Did we send them all ?
				if (hi2c->master.i2c_bytes_left-- == 0U) {
					// yup sent them all!

					// update the trb pointer
					hi2c->master.p_i2c1_trb_current++;

					// are we done with this string of requests?
					if (--hi2c->master.i2c1_trb_count == 0) {
						I2C_Master_Stop(hi2c, I2C_MESSAGE_COMPLETE);
					} else {
						// no!, there are more TRB to be sent.
						//I2C1_START_CONDITION_ENABLE_BIT = 1;

						// In some cases, the slave may require
						// a restart instead of a start. So use this one
						// instead.
						I2C_REPEAT_START_CONDITION_ENABLE_BIT = 1;

						// start the i2c request
						hi2c->master.i2c1_state = S_MASTER_SEND_ADDR;

					}
				} else {
					// Grab the next data to transmit
					I2C_TRANSMIT_REG = *hi2c->master.pi2c_buf_ptr++;
				}
			}
			break;

		case S_MASTER_ACK_ADDR:
			/* Make sure the previous byte was acknowledged */
			if (I2C_ACKNOWLEDGE_STATUS_BIT) {

				// Transmission was not acknowledged
				hi2c->master.i2cErrors++;

				// Send a stop flag and go back to idle
				I2C_Master_Stop(hi2c, I2C_MESSAGE_ADDRESS_NO_ACK);

				// Reset the Ack flag
				I2C_ACKNOWLEDGE_STATUS_BIT = 0;
			} else {
				I2C_RECEIVE_ENABLE_BIT = 1;
				hi2c->master.i2c1_state = S_MASTER_ACK_RCV_DATA;
			}
			break;

		case S_MASTER_RCV_DATA:

			/* Acknowledge is completed.  Time for more data */

			// Next thing is to ack the data
			hi2c->master.i2c1_state = S_MASTER_ACK_RCV_DATA;

			// Set up to receive a byte of data
			I2C_RECEIVE_ENABLE_BIT = 1;

			break;

		case S_MASTER_ACK_RCV_DATA:

			// Grab the byte of data received and acknowledge it
			*hi2c->master.pi2c_buf_ptr++ = I2C_RECEIVE_REG;

			// Check if we received them all?
			if (--hi2c->master.i2c_bytes_left) {

				/* No, there's more to receive */

				// No, bit 7 is clear.  Data is ok
				// Set the flag to acknowledge the data
				I2C_ACKNOWLEDGE_DATA_BIT = 0;

				// Wait for the acknowledge to complete, then get more
				hi2c->master.i2c1_state = S_MASTER_RCV_DATA;
			} else {

				// Yes, it's the last byte.  Don't ack it
				// Flag that we will nak the data
				I2C_ACKNOWLEDGE_DATA_BIT = 1;

				I2C_Master_FunctionComplete(hi2c);
			}

			// Initiate the acknowledge
			I2C_ACKNOWLEDGE_ENABLE_BIT = 1;
			break;

		case S_MASTER_RCV_STOP:
		case S_MASTER_SEND_STOP:

			// Send the stop flag
			I2C_Master_Stop(hi2c, I2C_MESSAGE_COMPLETE);
			break;

		default:

			// This case should not happen, if it does then
			// terminate the transfer
			hi2c->master.i2cErrors++;
			I2C_Master_Stop(hi2c, I2C_LOST_STATE);
			break;

	}
}

static void I2C_Master_FunctionComplete(I2C_HandleTypeDef *hi2c) {

	// update the trb pointer
	hi2c->master.p_i2c1_trb_current++;

	// are we done with this string of requests?
	if (--hi2c->master.i2c1_trb_count == 0) {
		hi2c->master.i2c1_state = S_MASTER_SEND_STOP;
	} else {
		hi2c->master.i2c1_state = S_MASTER_RESTART;
	}

}

static void I2C_Master_Stop(I2C_HandleTypeDef *hi2c, I2C_MESSAGE_STATUS completion_code) {
	// then send a stop
	I2C_STOP_CONDITION_ENABLE_BIT = 1;

	// make sure the flag pointer is not NULL
	if (hi2c->master.p_i2c1_current->pTrFlag != NULL)
	{
		// update the flag with the completion code
		*(hi2c->master.p_i2c1_current->pTrFlag) = completion_code;
	}

	// Done, back to idle
	hi2c->master.i2c1_state = S_MASTER_IDLE;

}

void I2C_Master_ReadTRBBuild(
	I2C_HandleTypeDef *hi2c,
	I2C_TRANSACTION_REQUEST_BLOCK *ptrb,
	uint8_t *pdata,
	uint8_t length,
	uint16_t address)
{
	ptrb->address  = address << 1;
	// make this a read
	ptrb->address |= 0x01;
	ptrb->length   = length;
	ptrb->pbuffer  = pdata;
}

void I2C_Master_WriteTRBBuild(I2C_HandleTypeDef *hi2c,
			      I2C_TRANSACTION_REQUEST_BLOCK *ptrb,
			      uint8_t *pdata,
			      uint8_t length,
			      uint16_t address)
{
	ptrb->address = address << 1;
	ptrb->length  = length;
	ptrb->pbuffer = pdata;
}

void I2C_Master_TRBInsert(
	I2C_HandleTypeDef *hi2c,
	uint8_t count,
	I2C_TRANSACTION_REQUEST_BLOCK *ptrb_list,
	I2C_MESSAGE_STATUS *pflag)
{

	// check if there is space in the queue
	if (hi2c->master.trStatus.s.full != true) {
		*pflag = I2C_MESSAGE_PENDING;

		hi2c->master.pTrTail->ptrb_list = ptrb_list;
		hi2c->master.pTrTail->count     = count;
		hi2c->master.pTrTail->pTrFlag   = pflag;
		hi2c->master.pTrTail++;

		// check if the end of the array is reached
		if (hi2c->master.pTrTail == (hi2c->master.i2c_tr_queue + hi2c->master.queue_len)) {
			// adjust to restart at the beginning of the array
			hi2c->master.pTrTail = hi2c->master.i2c_tr_queue;
		}

		// since we added one item to be processed, we know
		// it is not empty, so set the empty status to false
		hi2c->master.trStatus.s.empty = false;

		// check if full
		if (hi2c->master.pTrHead == hi2c->master.pTrTail) {
			// it is full, set the full status to true
			hi2c->master.trStatus.s.full = true;
		}

		// for interrupt based
		if (hi2c->master.i2c1_state == S_MASTER_IDLE) {
			// force the task to run since we know that the queue has
			// something that needs to be sent
			*hi2c->IFS |= 1U << hi2c->IFSOffset_M;
		}

	} else {
		*pflag = I2C_MESSAGE_FAIL;
	}

}

void I2C_Master_Write(
	I2C_HandleTypeDef *hi2c,
	uint8_t *pdata,
	uint8_t length,
	uint16_t address,
	I2C_MESSAGE_STATUS *pstatus) {
	I2C_TRANSACTION_REQUEST_BLOCK   trBlock;

	// check if there is space in the queue
	if (hi2c->master.trStatus.s.full != true) {
		I2C_Master_WriteTRBBuild(hi2c, &trBlock, pdata, length, address);
		I2C_Master_TRBInsert(hi2c, 1, &trBlock, pstatus);
	} else {
		*pstatus = I2C_MESSAGE_FAIL;
	}

}

void I2C_Master_Read(
	I2C_HandleTypeDef *hi2c,
	uint8_t *pdata,
	uint8_t length,
	uint16_t address,
	I2C_MESSAGE_STATUS *pstatus)
{
	I2C_TRANSACTION_REQUEST_BLOCK   trBlock;

	// check if there is space in the queue
	if (hi2c->master.trStatus.s.full != true) {
		I2C_Master_ReadTRBBuild(hi2c, &trBlock, pdata, length, address);
		I2C_Master_TRBInsert(hi2c, 1, &trBlock, pstatus);
	} else {
		*pstatus = I2C_MESSAGE_FAIL;
	}

}

bool I2C_Master_QueueIsEmpty(I2C_HandleTypeDef *hi2c) {
	return((bool)hi2c->master.trStatus.s.empty);
}

bool I2C_Master_QueueIsFull(I2C_HandleTypeDef *hi2c) {
	return((bool)hi2c->master.trStatus.s.full);
}

void i2cdetect(I2C_HandleTypeDef *hi2c) {
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

	for (int i = 0; i < 128; i += 16) {
		printf("%02x: ", i);
		for (int j = 0; j < 16; j++) {
			uint8_t buf;
			I2C_MESSAGE_STATUS rc;

			I2C_Master_Read(hi2c, &buf, 1, i + j, &rc);

			while (rc == I2C_MESSAGE_PENDING) {
				usleep(1000);
			}

			if (rc == I2C_MESSAGE_COMPLETE) {
				printf("%02x ", i+j);
			} else {
				printf("-- ");
			}
		}

		printf("\n");
	}
}

void i2cdump(I2C_HandleTypeDef *hi2c, uint16_t addr) {

	for (uint16_t i = 0; i < 256; i += 16) {
		printf("%02x: ", i);
		for (uint8_t j = 0; j < 16; j++) {
			uint8_t sub_addr = i+j;
			uint8_t buf;
			I2C_MESSAGE_STATUS rc_r, rc_w;

			I2C_Master_Write(hi2c, &sub_addr, 1, addr, &rc_w);

			while (rc_w == I2C_MESSAGE_PENDING) {
				usleep(1000);
			}

			if (rc_w == I2C_MESSAGE_COMPLETE) {
				I2C_Master_Read(hi2c, &buf, 1, addr, &rc_r);

				while (rc_r == I2C_MESSAGE_PENDING) {
					usleep(1000);
				}

				if (rc_r == I2C_MESSAGE_COMPLETE) {
					printf("%02x ", buf);
				} else {
					printf("X%d ", rc_r);
				}
			} else {
				printf("x%d ", rc_w);
			}
		}

		printf("\n");
	}
}

#endif