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


/** D E F I N I T I O N S ****************************************************/

#define CDC_COMM_IN_EP_SIZE	10
#define CDC_DATA_IN_EP_SIZE	64

/* Class-Specific Requests */
#define GET_NTB_PARAMETERS		0x80
#define GET_NTB_INPUT_SIZE		0x85
#define SET_NTB_INPUT_SIZE		0x86

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Notifications *
 * Note: Notifications are polled over
 * Communication Interface (Interrupt Endpoint)
 */
#define NETWORK_CONNECTION		0x00
#define RESPONSE_AVAILABLE		0x01
#define SERIAL_STATE			0x20
#define CONNECTION_SPEED_CHANGE		0x2a


/* Device Class Code */
#define CDC_DEVICE                  0x02

/* Communication Interface Class Code */
#define COMM_INTF                   0x02

/* Communication Interface Class SubClass Codes */
#define ABSTRACT_CONTROL_MODEL      0x02

/* Communication Interface Class Control Protocol Codes */
#define V25TER                      0x01    // Common AT commands ("Hayes(TM)")


/* Data Interface Class Codes */
#define DATA_INTF                   0x0A

/* Data Interface Class Protocol Codes */
#define NO_PROTOCOL                 0x00    // No class specific protocol required


/* Communication Feature Selector Codes */
#define ABSTRACT_STATE              0x01
#define COUNTRY_SETTING             0x02

/* Functional Descriptors */
/* Type Values for the bDscType Field */
#define CS_INTERFACE                0x24
#define CS_ENDPOINT                 0x25

/* bDscSubType in Functional Descriptors */
#define DSC_FN_HEADER               0x00
#define DSC_FN_CALL_MGT             0x01
#define DSC_FN_ACM                  0x02    // ACM - Abstract Control Management
#define DSC_FN_DLM                  0x03    // DLM - Direct Line Managment
#define DSC_FN_TELEPHONE_RINGER     0x04
#define DSC_FN_RPT_CAPABILITIES     0x05
#define DSC_FN_UNION                0x06
#define DSC_FN_COUNTRY_SELECTION    0x07
#define DSC_FN_TEL_OP_MODES         0x08
#define DSC_FN_USB_TERMINAL         0x09
/* more.... see Table 25 in USB CDC Specification 1.1 */

/* CDC Bulk IN transfer states */
#define CDC_TX_READY                0
#define CDC_TX_BUSY                 1
#define CDC_TX_BUSY_ZLP             2       // ZLP: Zero Length Packet
#define CDC_TX_COMPLETING           3

#define LINE_CODING_LENGTH          0x07

#if defined(USB_CDC_SET_LINE_CODING_HANDLER)
#define LINE_CODING_TARGET &cdc_notice.SetLineCoding._byte[0]
    #define LINE_CODING_PFUNC &USB_CDC_SET_LINE_CODING_HANDLER
#else
#define LINE_CODING_TARGET &cdc_ctx->line_coding._byte[0]
#define LINE_CODING_PFUNC NULL
#endif

#if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
#define CONFIGURE_RTS(a) UART_RTS = a;
#else
#define CONFIGURE_RTS(a)
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3)
#error This option is not currently supported.
#else
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2)
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL 0x04
#else
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1)
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL 0x02
#else
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0)
#error This option is not currently supported.
#else
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL 0x00
#endif

#define USB_CDC_ACM_FN_DSC_VAL  \
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL



#define USBDeluxe_CDC_BUF_SIZE			64
#define USBDeluxe_CDC_PKT_SIZE			64


typedef uint8_t USBDeluxeDevice_CDC_ECM_IOBuffer[USBDeluxe_CDC_BUF_SIZE];

/* Serial State Notification Packet Structure */
typedef struct {
	uint8_t    bmRequestType;  //Always 0xA1 for serial state notification packets
	uint8_t    bNotification;  //Always SERIAL_STATE (0x20) for serial state notification packets
	uint16_t  wValue;     //Always 0 for serial state notification packets
	uint16_t  wIndex;     //Interface number
	uint16_t  wLength;    //Should always be 2 for serial state notification packets
	uint8_t data[8];
} USBDeluxeDevice_CDC_State_Notification;

typedef struct {
	uint16_t (*RxDone)(void *userp, uint8_t *buf, uint16_t len);
	void (*TxDone)(void *userp);
} USBDeluxeDevice_CDC_ECM_IOps;

typedef struct {
	uint8_t *buf;
	uint16_t *buf_len;
} USBDeluxeDevice_RNDIS_UserBuffer;

typedef struct {
	USBDeluxeDevice_CDC_IOBuffer rx_buf[2];
	uint8_t rx_buf_len[2];
	uint8_t rx_buf_pos[2];

	USBDeluxeDevice_CDC_IOBuffer tx_buf[2];
	uint8_t tx_buf_len[2];

	uint8_t rx_buf_idx;
	uint8_t tx_buf_idx;

	void *userp;

	uint8_t USB_IFACE_COMM, USB_IFACE_DATA;
	uint8_t USB_EP_COMM, USB_EP_DATA;

	USBDeluxeDevice_CDC_ECM_IOps io_ops;

	USBDeluxeDevice_CDC_State_Notification cdc_notif;

	void *CDCDataOutHandle;
	void *CDCDataInHandle;
	void *CDCCommOutHandle;

	QueueHandle_t rx_queue;
	SemaphoreHandle_t tx_lock;

	uint8_t rx_queue_pending, rx_queue_pending_idx;

	Vector ntb_buffer;

	uint16_t header_seq;
	uint16_t header_total_length;
	uint16_t ndp_offset;

	uint16_t datagram_offset;
	uint16_t datagram_length;
} USBDeluxeDevice_CDCECMContext;

extern void USBDeluxeDevice_CDC_ECM_Create(USBDeluxeDevice_CDCECMContext *cdc_ctx, void *userp, uint8_t usb_iface_comm, uint8_t usb_iface_data, uint8_t usb_ep_comm, uint8_t usb_ep_data, USBDeluxeDevice_CDC_ECM_IOps *io_ops);
extern void USBDeluxeDevice_CDC_ECM_Init(USBDeluxeDevice_CDCECMContext *cdc_ctx);
extern void USBDeluxeDevice_CDC_ECM_CheckRequest(USBDeluxeDevice_CDCECMContext *cdc_ctx);
extern void USBDeluxeDevice_CDC_ECM_Tasks(USBDeluxeDevice_CDCECMContext *cdc_ctx);

extern int USBDeluxeDevice_CDC_ECM_AcquireRxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf);
extern void USBDeluxeDevice_CDC_ECM_AdvanceRxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx);
extern int USBDeluxeDevice_CDC_ECM_AcquireTxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf);
extern ssize_t USBDeluxeDevice_CDC_ECM_Read(USBDeluxeDevice_CDCECMContext *cdc_ctx, uint8_t *buf, size_t len);
extern ssize_t USBDeluxeDevice_CDC_ECM_Write(USBDeluxeDevice_CDCECMContext *cdc_ctx, uint8_t *buf, size_t len);

extern uint8_t USBDeluxe_DeviceFunction_Add_CDC_ECM(void *userp, const char *mac_addr, USBDeluxeDevice_CDC_ECM_IOps *io_ops);

