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

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
	union {
		struct {
			uint16_t TBF:1;
			uint16_t RBF:1;
			uint16_t R_NOT_W:1;
			uint16_t S:1;
			uint16_t P:1;
			uint16_t D_NOT_A:1;
			uint16_t I2COV:1;
			uint16_t IWCOL:1;
			uint16_t ADD10:1;
			uint16_t GCSTAT:1;
			uint16_t BCL:1;
			uint16_t :3;
			uint16_t TRSTAT:1;
			uint16_t ACKSTAT:1;
		};
		struct {
			uint16_t :2;
			uint16_t R_W:1;
			uint16_t :2;
			uint16_t D_A:1;
		};
	};
} I2CSTATBITS;

typedef struct {
	uint16_t SEN:1;
	uint16_t RSEN:1;
	uint16_t PEN:1;
	uint16_t RCEN:1;
	uint16_t ACKEN:1;
	uint16_t ACKDT:1;
	uint16_t STREN:1;
	uint16_t GCEN:1;
	uint16_t SMEN:1;
	uint16_t DISSLW:1;
	uint16_t A10M:1;
	uint16_t IPMIEN:1;
	uint16_t SCLREL:1;
	uint16_t I2CSIDL:1;
	uint16_t :1;
	uint16_t I2CEN:1;
} I2CCONBITS;

typedef enum {
	I2C_MESSAGE_FAIL,
	I2C_MESSAGE_PENDING,
	I2C_MESSAGE_COMPLETE,
	I2C_STUCK_START,
	I2C_MESSAGE_ADDRESS_NO_ACK,
	I2C_DATA_NO_ACK,
	I2C_LOST_STATE
} I2C_MESSAGE_STATUS;

/**
  I2C Master Driver State Enumeration

  @Summary
    Defines the different states of the i2c master.

  @Description
    This defines the different states that the i2c master
    used to process transactions on the i2c bus.
*/

typedef enum {
	S_MASTER_IDLE,
	S_MASTER_RESTART,
	S_MASTER_SEND_ADDR,
	S_MASTER_SEND_DATA,
	S_MASTER_SEND_STOP,
	S_MASTER_ACK_ADDR,
	S_MASTER_RCV_DATA,
	S_MASTER_RCV_STOP,
	S_MASTER_ACK_RCV_DATA,
	S_MASTER_NOACK_STOP,
	S_MASTER_SEND_ADDR_10BIT_LSB,
	S_MASTER_10BIT_RESTART,

} I2C_MASTER_STATES;

/**
  I2C Driver Transaction Request Block (TRB) type definition.

  @Summary
    This defines the Transaction Request Block (TRB) used by the
    i2c master in sending/receiving data to the i2c bus.

  @Description
    This data type is the i2c Transaction Request Block (TRB) that
    the needs to be built and sent to the driver to handle each i2c requests.
    Using the TRB, simple to complex i2c transactions can be constructed
    and sent to the i2c bus. This data type is only used by the master mode.

 */
typedef struct
{
	uint16_t  address;          // Bits <10:1> are the 10 bit address.
	// Bits <7:1> are the 7 bit address
	// Bit 0 is R/W (1 for read)
	uint8_t   length;           // the # of bytes in the buffer
	uint8_t   *pbuffer;         // a pointer to a buffer of length bytes
} I2C_TRANSACTION_REQUEST_BLOCK;

/**
  I2C Driver Queue Status Type

  @Summary
    Defines the type used for the transaction queue status.

  @Description
    This defines type used to keep track of the queue status.
 */

typedef union {
	struct {
		uint8_t full:1;
		uint8_t empty:1;
		uint8_t reserved:6;
	} s;
	uint8_t status;
} I2C_TR_QUEUE_STATUS;

/**
  I2C Driver Queue Entry Type

  @Summary
    Defines the object used for an entry in the i2c queue items.

  @Description
    This defines the object in the i2c queue. Each entry is a composed
    of a list of TRBs, the number of the TRBs and the status of the
    currently processed TRB.
 */
typedef struct {
	uint8_t                         count;          // a count of trb's in the trb list
	I2C_TRANSACTION_REQUEST_BLOCK  *ptrb_list;     // pointer to the trb list
	I2C_MESSAGE_STATUS             *pTrFlag;       // set with the error of the last trb sent.
	// if all trb's are sent successfully,
	// then this is I2C1_MESSAGE_COMPLETE
} I2C_TR_QUEUE_ENTRY;

/**
  I2C Master Driver Object Type

  @Summary
    Defines the object that manages the i2c master.

  @Description
    This defines the object that manages the sending and receiving of
    i2c master transactions.
  */

typedef struct {
	I2C_TR_QUEUE_ENTRY		*i2c_tr_queue;

	I2C_MASTER_STATES		i2c1_state;

	I2C_TRANSACTION_REQUEST_BLOCK	*p_i2c1_trb_current;
	I2C_TR_QUEUE_ENTRY		*p_i2c1_current;

	I2C_TR_QUEUE_ENTRY *pTrTail;		// tail of the queue
	I2C_TR_QUEUE_ENTRY *pTrHead;		// head of the queue
	I2C_TR_QUEUE_STATUS trStatus;		// status of the last transaction

	uint8_t  *pi2c_buf_ptr;
	uint16_t i2c_address;

	uint8_t	i2c1_trb_count;
	uint8_t	queue_len;

	uint8_t i2cDoneFlag;			// flag to indicate the current, transaction is done
	uint8_t i2cErrors;			// keeps track of errors

	uint8_t i2c_bytes_left;
	uint8_t i2c_10bit_address_restart;
} I2C_MASTER_OBJECT;

typedef struct {
	union {
		I2C_MASTER_OBJECT master;
	};

	volatile I2CSTATBITS	*STAT;
	volatile I2CCONBITS 	*CON;
	volatile uint16_t	*BRG;
	volatile uint16_t	*TRN;
	volatile uint16_t	*RCV;

	volatile uint16_t	*IEC;
	volatile uint16_t	*IFS;
	uint8_t		IECOffset_M;
	uint8_t		IECOffset_S;
	uint8_t		IFSOffset_M;
	uint8_t		IFSOffset_S;

	uint8_t 	mode;
} I2C_HandleTypeDef;

enum {
	I2C_7BIT = 0x0,
	I2C_10BIT = 0x1,
};

enum {
	I2C_B100000 = 0x9D,
	I2C_B400000 = 0x25,
	I2C_B1000000 = 0xD,
};

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

extern void I2C_Master_Initialize(I2C_HandleTypeDef *hi2c, uint8_t i2c_mode, uint8_t queue_len, uint16_t speed);
extern void I2C_Master_ProcessInterrupt(I2C_HandleTypeDef *hi2c);
extern void I2C_Master_Read(I2C_HandleTypeDef *hi2c, uint8_t *pdata, uint8_t length, uint16_t address, I2C_MESSAGE_STATUS *pstatus);
extern void I2C_Master_Write(I2C_HandleTypeDef *hi2c, uint8_t *pdata, uint8_t length, uint16_t address, I2C_MESSAGE_STATUS *pstatus);

extern void i2cdetect(I2C_HandleTypeDef *hi2c);
extern void i2cdump(I2C_HandleTypeDef *hi2c, uint16_t addr);