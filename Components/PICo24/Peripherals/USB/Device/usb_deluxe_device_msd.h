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


    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright 2015 Microchip Technology Inc. (www.microchip.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <stdint.h>

#define MSD_IN_EP_SIZE          64u
#define MSD_OUT_EP_SIZE         64u

#define MSD_FAILED_READ_MAX_ATTEMPTS	100
#define MSD_FAILED_WRITE_MAX_ATTEMPTS	100


/** D E F I N I T I O N S ****************************************************/
#define FILEIO_CONFIG_MEDIA_SECTOR_SIZE 512

/* MSD Interface Class Code */
#define MSD_INTF                    0x08

/* MSD Interface Class SubClass Codes */
//Options - from usb_msc_overview_1[1].2.pdf
//  Supported
#define SCSI_TRANSPARENT    0x06
//  Not-Supported
#define RBC                 0x01    // Reduced Block Commands (RBC) T10 Project 1240-D
#define SSF_8020i           0x02    // C/DVD devices typically use SSF-8020i or MMC-2
#define MMC_2               0x02
#define QIC_157             0x03    // Tape drives typically use QIC-157 command blocks
#define UFI                 0x04    // Typically a floppy disk drive (FDD) device
#define SSF_8070i           0x05    // Typically a floppy disk drive uses SSF-8070i commands

#define MSD_INTF_SUBCLASS          SCSI_TRANSPARENT
//#define MSD_INTF_SUBCLASS          RBC

/* MSD Interface Class Protocol Codes */
#define MSD_PROTOCOL           0x50

/* Class Commands */
#define MSD_RESET 0xff
#define GET_MAX_LUN 0xfe

#define BLOCKLEN_512                0x0200

#define STMSDTRIS TRISD0
#define STRUNTRIS TRISD1
#define STMSDLED LATDbits.LATD0
#define STRUNLED LATDbits.LATD1
#define ToggleRUNLED() STRUNLED = !STRUNLED;

//**********************************************************DOM-IGNORE-BEGIN
//Various States of Mass Storage Firmware (MSDTasks)
//**********************************************************DOM-IGNORE-END

//MSD_WAIT is when the MSD state machine is idle (returned by MSDTasks())
#define MSD_WAIT                            0x00
//MSD_DATA_IN is when the device is sending data (returned by MSDTasks())
#define MSD_DATA_IN                         0x01
//MSD_DATA_OUT is when the device is receiving data (returned by MSDTasks())
#define MSD_DATA_OUT                        0x02
//MSD_SEND_CSW is when the device is waiting to send the CSW (returned by MSDTasks())
#define MSD_SEND_CSW                        0x03

//States of the MSDProcessCommand state machine
#define MSD_COMMAND_WAIT                    0xFF
#define MSD_COMMAND_ERROR                   0xFE
#define MSD_COMMAND_RESPONSE                0xFD
#define MSD_COMMAND_RESPONSE_SEND           0xFC
#define MSD_COMMAND_STALL                   0xFB

/* SCSI Transparent Command Set Sub-class code */
#define MSD_INQUIRY                     	0x12
#define MSD_READ_FORMAT_CAPACITY         	0x23
#define MSD_READ_CAPACITY                 	0x25
#define MSD_READ_10                     	0x28
#define MSD_WRITE_10                     	0x2a
#define MSD_REQUEST_SENSE                 	0x03
#define MSD_MODE_SENSE                  	0x1a
#define MSD_PREVENT_ALLOW_MEDIUM_REMOVAL 	0x1e
#define MSD_TEST_UNIT_READY             	0x00
#define MSD_VERIFY                         	0x2f
#define MSD_STOP_START                     	0x1b

#define MSD_READ10_WAIT                     0x00
#define MSD_READ10_BLOCK                    0x01
#define MSD_READ10_SECTOR                   0x02
#define MSD_READ10_TX_SECTOR                0x03
#define MSD_READ10_TX_PACKET                0x04
#define MSD_READ10_FETCH_DATA               0x05
#define MSD_READ10_XMITING_DATA             0x06
#define MSD_READ10_AWAITING_COMPLETION      0x07
#define MSD_READ10_ERROR                    0xFF

#define MSD_WRITE10_WAIT                    0x00
#define MSD_WRITE10_BLOCK                   0x01
#define MSD_WRITE10_SECTOR                  0x02
#define MSD_WRITE10_RX_SECTOR               0x03
#define MSD_WRITE10_RX_PACKET               0x04

//Define MSD_USE_BLOCKING in order to block the code in an
//attempt to get better throughput.
//#define MSD_USE_BLOCKING

#define MSD_CSW_SIZE    0x0d	// 10 bytes CSW data
#define MSD_CBW_SIZE    0x1f	// 31 bytes CBW data
#define MSD_MAX_CB_SIZE 0x10    //MSD BOT Command Block (CB) size is 16 bytes maximum (bytes 0x0F-0x1E in the CBW)
#define MSD_CBWFLAGS_RESERVED_BITS_MASK    0x3F //Bits 5..0 of the bCBWFlags uint8_t are reserved, and should be set = 0 by host in order for CBW to be considered meaningful

#define MSD_VALID_CBW_SIGNATURE (uint32_t)0x43425355
#define MSD_VALID_CSW_SIGNATURE (uint32_t)0x53425355

//MSDErrorHandler() definitions, see section 6.7 of BOT specifications revision 1.0
//Note: We re-use values for some of the cases.  This is because the error handling
//behavior is the same for some of the different test case numbers.
#define MSD_ERROR_CASE_NO_ERROR         0x00
#define MSD_ERROR_CASE_2                 0x01
#define	MSD_ERROR_CASE_3                 0x01
#define MSD_ERROR_CASE_4                 0x02
#define	MSD_ERROR_CASE_5                 0x02
#define MSD_ERROR_CASE_7                 0x03
#define	MSD_ERROR_CASE_8                 0x03
#define MSD_ERROR_CASE_9                 0x04
#define MSD_ERROR_CASE_11               0x04
#define	MSD_ERROR_CASE_10               0x05
#define	MSD_ERROR_CASE_13               0x05
#define MSD_ERROR_UNSUPPORTED_COMMAND   0x7F


#define INVALID_CBW 1
#define VALID_CBW !INVALID_CBW

/* Sense Key Error Codes */
//------------------------------------------------------------------------------
#define S_NO_SENSE 0x0
#define S_RECOVERED_ERROR 0x1
#define S_NOT_READY 0x2
#define S_MEDIUM_ERROR 0x3
#define S_HARDWARE_ERROR 0X4
#define S_ILLEGAL_REQUEST 0x5
#define S_UNIT_ATTENTION 0x6
#define S_DATA_PROTECT 0x7
#define S_BLANK_CHECK 0x8
#define S_VENDOR_SPECIFIC 0x9
#define S_COPY_ABORTED 0xa
#define S_ABORTED_COMMAND 0xb
#define S_OBSOLETE 0xc
#define S_VOLUME_OVERFLOW 0xd
#define S_MISCOMPARE 0xe

#define S_CURRENT 0x70
#define S_DEFERRED 0x71

//------------------------------------------------------------------------------
//ASC/ASCQ Codes for Sense Data (only those that we plan to use):
//The ASC/ASCQ code expand on the information provided in the sense key error
//code, and provide the host with more details about the specific issue/status.
//------------------------------------------------------------------------------
//For use with sense key Illegal request for a command not supported
#define ASC_NO_ADDITIONAL_SENSE_INFORMATION 0x00
#define ASCQ_NO_ADDITIONAL_SENSE_INFORMATION 0x00

#define ASC_INVALID_COMMAND_OPCODE 0x20
#define ASCQ_INVALID_COMMAND_OPCODE 0x00

// from SPC-3 Table 185
// with sense key Illegal Request for test unit ready
#define ASC_LOGICAL_UNIT_NOT_SUPPORTED 0x25
#define ASCQ_LOGICAL_UNIT_NOT_SUPPORTED 0x00

//For use with sense key Not ready
#define ASC_LOGICAL_UNIT_DOES_NOT_RESPOND 0x05
#define ASCQ_LOGICAL_UNIT_DOES_NOT_RESPOND 0x00

//For use with S_UNIT_ATTENTION
#define ASC_NOT_READY_TO_READY_CHANGE    0x28
#define ASCQ_MEDIUM_MAY_HAVE_CHANGED    0x00

#define ASC_MEDIUM_NOT_PRESENT 0x3a
#define ASCQ_MEDIUM_NOT_PRESENT 0x00

#define ASC_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE 0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_CAUSE_NOT_REPORTABLE 0x00

#define ASC_LOGICAL_UNIT_IN_PROCESS 0x04
#define ASCQ_LOGICAL_UNIT_IN_PROCESS 0x01

#define ASC_LOGICAL_UNIT_NOT_READY_INIT_REQD 0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_INIT_REQD 0x02

#define ASC_LOGICAL_UNIT_NOT_READY_INTERVENTION_REQD 0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_INTERVENTION_REQD 0x03

#define ASC_LOGICAL_UNIT_NOT_READY_FORMATTING 0x04
#define ASCQ_LOGICAL_UNIT_NOT_READY_FORMATTING 0x04

#define ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x21
#define ASCQ_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x00

#define ASC_WRITE_PROTECTED 0x27
#define ASCQ_WRITE_PROTECTED 0x00


//Possible command status codes returned in the Command Status Wrapper (CSW)
#define MSD_CSW_COMMAND_PASSED  0x00
#define MSD_CSW_COMMAND_FAILED  0x01
#define MSD_CSW_PHASE_ERROR     0x02

#define MSD_CBW_DIRECTION_BITMASK   0x80


typedef union {
	uint8_t v[4];
	uint16_t w[2];
	uint32_t Val;
} USBDeluxeDevice_MSD_LBA;

typedef union {
	uint8_t v[4];
	uint16_t w[2];
	uint32_t Val;
} USBDeluxeDevice_MSD_BLK;

typedef union {
	uint8_t v[4];
	uint16_t w[2];
	uint32_t Val;
} USBDeluxeDevice_MSD_SECTOR_SIZE;

typedef union {
	uint8_t v[4];
	uint16_t w[2];
	uint32_t Val;
} USBDeluxeDevice_MSD_CAPACITY;

typedef union {
	uint8_t v[2];
	struct {
		uint8_t LB;
		uint8_t HB;
	} __attribute__((packed)) byte;
	uint16_t Val;
} USBDeluxeDevice_MSD_TRANSFER_LENGTH;

typedef union {
	uint8_t v[4];
	uint16_t w[2];
	uint32_t Val;
} USBDeluxeDevice_MSD_CMD_SPECIFIC_INFO;

typedef struct { //31 bytes total Command Block Wrapper
	uint32_t dCBWSignature;            // 55 53 42 43h
	uint32_t dCBWTag;                    // sent by host, device echos this value in CSW (associated a CSW with a CBW)
	uint32_t dCBWDataTransferLength;     // number of bytes of data host expects to transfer
	uint8_t bCBWFlags;                 // CBW flags, bit 7 = 0-data out from host to device,
	//                    = 1-device to host, rest bits 0
	uint8_t bCBWLUN;                    // Most Significant 4bits are always zero, 0 in our case as only one logical unit
	uint8_t bCBWCBLength;                // Here most significant 3bits are zero
	uint8_t CBWCB[16];                    // Command block to be executed by the device
} USBDeluxeDevice_MSD_CBW;

typedef struct { // Command Status Wrapper
	uint32_t dCSWSignature;            // 55 53 42 53h Signature of a CSW packet
	uint32_t dCSWTag;                    // echo the dCBWTag of the CBW packet
	uint32_t dCSWDataResidue;            // difference in data expected (dCBWDataTransferLength) and actual amount processed/sent
	uint8_t bCSWStatus;                // 00h Command Passed, 01h Command Failed, 02h Phase Error, rest obsolete/reserved
} USBDeluxeDevice_MSD_CSW;

/* Fixed format if Desc bit of request sense cbw is 0 */
typedef union {
	struct {
		uint8_t _byte[18];
	};

	struct {
		unsigned ResponseCode:7;            // b6-b0 is Response Code Fixed or descriptor format
		unsigned VALID:1;                    // Set to 1 to indicate information field is a valid value

		uint8_t Obsolete;

		unsigned SenseKey:4;                // Refer SPC-3 Section 4.5.6
		unsigned Resv:1;
		unsigned ILI:1;                        // Incorrect Length Indicator
		unsigned EOM:1;                        // End of Medium
		unsigned FILEMARK:1;                 // for READ and SPACE commands

		uint8_t InformationB0;                    // device type or command specific (SPC-33.1.18)
		uint8_t InformationB1;                    // device type or command specific (SPC-33.1.18)
		uint8_t InformationB2;                    // device type or command specific (SPC-33.1.18)
		uint8_t InformationB3;                    // device type or command specific (SPC-33.1.18)
		uint8_t AddSenseLen;                    // number of additional sense bytes that follow <=244
		USBDeluxeDevice_MSD_CMD_SPECIFIC_INFO CmdSpecificInfo;                // depends on command on which exception occurred
		uint8_t ASC;                            // additional sense code
		uint8_t ASCQ;                            // additional sense code qualifier Section 4.5.2.1 SPC-3
		uint8_t FRUC;                            // Field Replaceable Unit Code 4.5.2.5 SPC-3

		uint8_t SenseKeySpecific[3];            // msb is SKSV sense-key specific valid field set=> valid SKS
		// 18-n additional sense bytes can be defined later
		// 18 Bytes Request Sense Fixed Format
	} __attribute__((packed));
} __attribute__((packed)) USBDeluxeDevice_MSD_RequestSenseResponse;

typedef struct {
	uint8_t Peripheral;                 // Peripheral_Qualifier:3; Peripheral_DevType:5;
	uint8_t Removble;                   // removable medium bit7 = 0 means non removable, rest reserved
	uint8_t Version;                    // version
	uint8_t Response_Data_Format;       // b7,b6 Obsolete, b5 Access control co-ordinator, b4 hierarchical addressing support
	// b3:0 response data format 2 indicates response is in format defined by spec
	uint8_t AdditionalLength;           // length in bytes of remaining in standard inquiry data
	uint8_t Sccstp;                     // b7 SCCS, b6 ACC, b5-b4 TGPS, b3 3PC, b2-b1 Reserved, b0 Protected
	uint8_t bqueetc;                    // b7 bque, b6- EncServ, b5-VS, b4-MultiP, b3-MChngr, b2-b1 Obsolete, b0-Addr16
	uint8_t CmdQue;                     // b7-b6 Obsolete, b5-WBUS, b4-Sync, b3-Linked, b2 Obsolete,b1 Cmdque, b0-VS
	char vendorID[8];
	char productID[16];
	char productRev[4];
} USBDeluxeDevice_MSD_InquiryResponse;      //36 bytes total

typedef struct {
	uint8_t ModeDataLen;
	uint8_t MediumType;
	unsigned Resv:4;
	unsigned DPOFUA:1;                    // 0 indicates DPO and FUA bits not supported
	unsigned notused:2;
	unsigned WP:1;                        // 0 indicates not write protected
	uint8_t BlockDscLen;                    // Block Descriptor Length
} USBDeluxeDevice_MSD_tModeParamHdr;

/* Short LBA mode block descriptor (see Page 1009, SBC-2) */
typedef struct {
	uint8_t NumBlocks[4];
	uint8_t Resv;                            // reserved
	uint8_t BlockLen[3];
} USBDeluxeDevice_MSD_tBlockDescriptor;

/* Page_0 mode page format */
typedef struct {
	unsigned PageCode:6;                // SPC-3 7.4.5
	unsigned SPF:1;                        // SubPageFormat=0 means Page_0 format
	unsigned PS:1;                        // Parameters Saveable

	uint8_t PageLength;                    // if 2..n bytes of mode parameters PageLength = n-1
	uint8_t ModeParam[];                    // mode parameters
} USBDeluxeDevice_MSD_tModePage;

typedef struct {
	uint8_t (*MediaInitialize)(void *userp, uint8_t lun_idx);
	uint8_t (*MediaDetect)(void *userp, uint8_t lun_idx);

	uint32_t (*ReadCapacity)(void *userp, uint8_t lun_idx);
	uint16_t  (*ReadSectorSize)(void *userp, uint8_t lun_idx);

	uint8_t (*WriteProtectState)(void *userp, uint8_t lun_idx);

	uint8_t (*SectorRead)(void *userp, uint8_t lun_idx, uint32_t sector_addr, uint8_t* buffer);
	uint8_t (*SectorWrite)(void *userp, uint8_t lun_idx, uint32_t sector_addr, uint8_t* buffer, uint8_t allowWriteToZero);
} USBDeluxeDevice_MSD_DiskOps;

typedef struct {
	uint8_t USB_IFACE;
	uint8_t USB_EP_OUT, USB_EP_IN;

	uint8_t nr_LUNs;

	void *userp;

	const char *vendor_id;
	const char *product_id;
	const char *product_rev;

	uint8_t MSD_State;			// Takes values MSD_WAIT, MSD_DATA_IN or MSD_DATA_OUT
	uint8_t MSDCommandState;
	uint8_t MSDReadState;
	uint8_t MSDWriteState;
	uint8_t MSDRetryAttempt;
	bool MSDCBWValid;

	uint8_t msd_buffer[512];
	uint8_t *ptrNextData;

	USBDeluxeDevice_MSD_LBA LBA;
	USBDeluxeDevice_MSD_TRANSFER_LENGTH TransferLength;

	USBDeluxeDevice_MSD_CBW msd_cbw;
	USBDeluxeDevice_MSD_CSW msd_csw;

	uint16_t gblMediaPresent;		// Bit array

	USBDeluxeDevice_MSD_RequestSenseResponse *gblSenseData;
	USBDeluxeDevice_MSD_CBW gblCBW;
	USBDeluxeDevice_MSD_BLK gblNumBLKS, gblBLKLen;

	void *USBMSDOutHandle;
	void *USBMSDInHandle;

	USBDeluxeDevice_MSD_DiskOps diskops;

	bool MSDHostNoData;
} USBDeluxeDevice_MSDContext;

extern void USBDeluxeDevice_MSD_Create(USBDeluxeDevice_MSDContext *msd_ctx, void *userp, uint8_t usb_iface, uint8_t usb_ep_in, uint8_t usb_ep_out,
				       uint8_t nr_luns, const char *vendor_id, const char *product_id, const char *product_rev, USBDeluxeDevice_MSD_DiskOps *diskops);
extern void USBDeluxeDevice_MSD_Init(USBDeluxeDevice_MSDContext *msd_ctx);
extern uint8_t USBDeluxeDevice_MSD_Tasks(USBDeluxeDevice_MSDContext *msd_ctx);
extern void USBDeluxeDevice_MSD_CheckRequest(USBDeluxeDevice_MSDContext *msd_ctx);

extern uint8_t USBDeluxe_DeviceFunction_Add_MSD(void *userp, uint8_t nr_luns, const char *vendor_id, const char *product_id, const char *product_rev, USBDeluxeDevice_MSD_DiskOps *disk_ops);
