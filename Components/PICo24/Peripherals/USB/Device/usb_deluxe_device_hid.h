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

/* Class-Specific Requests */
#define GET_REPORT      0x01
#define GET_IDLE        0x02
#define GET_PROTOCOL    0x03
#define SET_REPORT      0x09
#define SET_IDLE        0x0A
#define SET_PROTOCOL    0x0B

/* Class Descriptor Types */
#define DSC_HID         0x21
#define DSC_RPT         0x22
#define DSC_PHY         0x23

/* Protocol Selection */
#define BOOT_PROTOCOL   0x00
#define RPT_PROTOCOL    0x01

/* HID Interface Class Code */
#define HID_INTF                    0x03

/* HID Interface Class SubClass Codes */
#define BOOT_INTF_SUBCLASS          0x01

/* HID Interface Class Protocol Codes */
#define HID_PROTOCOL_NONE           0x00
#define HID_PROTOCOL_KEYBOARD       0x01
#define HID_PROTOCOL_MOUSE          0x02

// Section: STRUCTURES *********************************************/

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC_HEADER
{
	uint8_t bDescriptorType;	//offset 9
	uint16_t wDscLength;		//offset 10
} USB_HID_DSC_HEADER;

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC
{
	uint8_t bLength;			//offset 0
	uint8_t bDescriptorType;	//offset 1
	uint16_t bcdHID;			//offset 2
	uint8_t bCountryCode;		//offset 4
	uint8_t bNumDsc;			//offset 5


	//USB_HID_DSC_HEADER hid_dsc_header[HID_NUM_OF_DSC];
	/* HID_NUM_OF_DSC is defined in usbcfg.h */

} USB_HID_DSC;

typedef struct {
	void (*GetReport)(void *userp);
	void (*SetReport)(void *userp);
	void (*SetIdleRate)(void *userp, uint8_t idle_rate);
	void (*GetReportDescriptor)(void *userp, uint8_t *buf);
	void (*RxDone)(void *userp, uint8_t *buf, uint16_t len);
} USBDeluxeDevice_HID_Operations;

typedef struct {
	void *userp;

	uint8_t buffer[64];

	uint8_t USB_IFACE, USB_EP;

	uint16_t hid_dsc_offset, hid_dsc_len;
	uint8_t hid_rpt_len, hid_rpt_dsc_len;

	USB_HANDLE *HandleRx, *HandleTx;
	uint8_t idle_rate, active_protocol;

	USBDeluxeDevice_HID_Operations ops;
} USBDeluxeDevice_HIDContext;

extern void USBDeluxeDevice_HID_Create(USBDeluxeDevice_HIDContext *hid_ctx, void *userp,
				       uint8_t usb_iface, uint8_t usb_ep,
				       uint8_t hid_rpt_len, uint8_t hid_rpt_dsc_len,
				       uint16_t hid_dsc_offset, uint16_t hid_dsc_len,
				       USBDeluxeDevice_HID_Operations *hid_ops);
extern void USBDeluxeDevice_HID_Init(USBDeluxeDevice_HIDContext *hid_ctx);
extern void USBDeluxeDevice_HID_CheckRequest(USBDeluxeDevice_HIDContext *hid_ctx);
extern void USBDeluxeDevice_HID_Tasks(USBDeluxeDevice_HIDContext *hid_ctx);
extern int USBDeluxeDevice_HID_Write(USBDeluxeDevice_HIDContext *hid_ctx, uint8_t *buf, uint8_t len);

extern uint8_t USBDeluxe_DeviceFunction_Add_HID(void *userp, uint8_t hid_protocol, uint8_t hid_report_size, uint8_t hid_report_desc_size, USBDeluxeDevice_HID_Operations *hid_ops);
