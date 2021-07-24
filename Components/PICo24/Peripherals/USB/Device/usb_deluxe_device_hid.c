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

#include "usb_deluxe_device.h"
#include "usb_deluxe_device_hid.h"

extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID

void USBDeluxeDevice_HID_Create(USBDeluxeDevice_HIDContext *hid_ctx, void *userp,
				uint8_t usb_iface, uint8_t usb_ep,
				uint8_t hid_rpt_len, uint8_t hid_rpt_dsc_len,
				uint16_t hid_dsc_offset, uint16_t hid_dsc_len,
				USBDeluxeDevice_HID_Operations *hid_ops) {
	memset(hid_ctx, 0, sizeof(USBDeluxeDevice_HIDContext));

	hid_ctx->userp = userp;
	hid_ctx->USB_IFACE = usb_iface;
	hid_ctx->USB_EP = usb_ep;
	hid_ctx->hid_dsc_offset = hid_dsc_offset;
	hid_ctx->hid_dsc_len = hid_dsc_len;
	hid_ctx->hid_rpt_len = hid_rpt_len;
	hid_ctx->hid_rpt_dsc_len = hid_rpt_dsc_len;

	memcpy(&hid_ctx->ops, hid_ops, sizeof(USBDeluxeDevice_HID_Operations));
}

void USBDeluxeDevice_HID_Init(USBDeluxeDevice_HIDContext *hid_ctx) {
	USBEnableEndpoint(hid_ctx->USB_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	USBRxOnePacket(hid_ctx->USB_EP, hid_ctx->buffer, sizeof(hid_ctx->buffer));
}

void USBDeluxeDevice_HID_CheckRequest(USBDeluxeDevice_HIDContext *hid_ctx) {
	if (SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;
	if (SetupPkt.bIntfID != hid_ctx->USB_IFACE) return;

	/*
	 * There are two standard requests that hid.c may support.
	 * 1. GET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
	 * 2. SET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
	 */
	if(SetupPkt.bRequest == USB_REQUEST_GET_DESCRIPTOR) {
		switch(SetupPkt.bDescriptorType) {
			case DSC_HID: //HID Descriptor
				if (USBActiveConfiguration == 1) {
					USBEP0SendROMPtr(usb_device_desc_ctx.raw.data + hid_ctx->hid_dsc_offset,hid_ctx->hid_dsc_len, USB_EP0_INCLUDE_ZERO);
				}
				break;
			case DSC_RPT:  //Report Descriptor
				//if(USBActiveConfiguration == 1)
			{
				if (hid_ctx->ops.GetReportDescriptor) {
					hid_ctx->ops.GetReportDescriptor(hid_ctx->userp, hid_ctx->buffer);
					USBEP0SendROMPtr(hid_ctx->buffer, hid_ctx->hid_rpt_dsc_len, USB_EP0_INCLUDE_ZERO);
				}

			}
				break;
			case DSC_PHY:  //Physical Descriptor
				//Note: The below placeholder code is commented out.  HID Physical Descriptors are optional and are not used
				//in many types of HID applications.  If an application does not have a physical descriptor,
				//then the device should return STALL in response to this request (stack will do this automatically
				//if no-one claims ownership of the control transfer).
				//If an application does implement a physical descriptor, then make sure to declare
				//hid_phy01 (rom structure containing the descriptor data), and hid_phy01 (the size of the descriptors in uint8_ts),
				//and then uncomment the below code.
				//if(USBActiveConfiguration == 1)
				//{
				//    USBEP0SendROMPtr((const uint8_t*)&hid_phy01, sizeof(hid_phy01), USB_EP0_INCLUDE_ZERO);
				//}
				break;
		}//end switch(SetupPkt.bDescriptorType)
	}//end if(SetupPkt.bRequest == GET_DSC)

	if (SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) {
		return;
	}

	switch (SetupPkt.bRequest) {
		case GET_REPORT:
			if (hid_ctx->ops.GetReport) {
				hid_ctx->ops.GetReport(hid_ctx->userp);
			}
			break;
		case SET_REPORT:
			if (hid_ctx->ops.SetReport) {
				hid_ctx->ops.SetReport(hid_ctx->userp);
			}
			break;
		case GET_IDLE:
		USBEP0SendRAMPtr((uint8_t*)&hid_ctx->idle_rate, 1, USB_EP0_INCLUDE_ZERO);
			break;
		case SET_IDLE:
			USBEP0Transmit(USB_EP0_NO_DATA);
			hid_ctx->idle_rate = SetupPkt.W_Value.byte.HB;

			if (hid_ctx->ops.SetIdleRate) {
				hid_ctx->ops.SetIdleRate(hid_ctx->userp, hid_ctx->idle_rate);
			}

			break;
		case GET_PROTOCOL:
		USBEP0SendRAMPtr((uint8_t*)&hid_ctx->active_protocol, 1, USB_EP0_NO_OPTIONS);
			break;
		case SET_PROTOCOL:
			USBEP0Transmit(USB_EP0_NO_DATA);
			hid_ctx->active_protocol = SetupPkt.W_Value.byte.LB;
			break;
	}//end switch(SetupPkt.bRequest)

}//end USBCheckHIDRequest

int USBDeluxeDevice_HID_Write(USBDeluxeDevice_HIDContext *hid_ctx, uint8_t *buf, uint8_t len) {
	if (USBHandleBusy(hid_ctx->HandleTx)) {
		return -1;
	}

	hid_ctx->HandleTx = USBTxOnePacket(hid_ctx->USB_EP, buf, len);

	return 0;
}

void USBDeluxeDevice_HID_Tasks(USBDeluxeDevice_HIDContext *hid_ctx) {
	if (USBHandleBusy(hid_ctx->HandleRx)) {
		return;
	} else {
		uint8_t len = USBHandleGetLength(hid_ctx->HandleRx);
		if (hid_ctx->ops.RxDone) {
			hid_ctx->ops.RxDone(hid_ctx->userp, hid_ctx->buffer, len);
		}
	}

	USBRxOnePacket(hid_ctx->USB_EP, hid_ctx->buffer, sizeof(hid_ctx->buffer));
}

void USBDeluxe_DeviceDescriptor_InsertHIDSpecific(uint16_t version, uint8_t country, uint8_t nr_dscs, uint16_t report_size) {
	/* HID Class-Specific Descriptor */
	uint8_t buf0[] = {
		0x9,						// Size of this descriptor in bytes
		DSC_HID,					// HID descriptor type
		version & 0xff,					// HID Spec Release Number in BCD format <7:0>
		version >> 8,					// HID Spec Release Number in BCD format <15:8>
		country,					// Country Code
		nr_dscs,					// Number of class descriptors
		DSC_RPT,					// Report descriptor type
		report_size & 0xff,				// Size of the report descriptor <7:0>
		report_size >> 8,				// Size of the report descriptor <15:8>
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

uint8_t USBDeluxe_DeviceFunction_Add_HID(void *userp, uint8_t hid_protocol, uint8_t hid_report_size, uint8_t hid_report_desc_size, USBDeluxeDevice_HID_Operations *hid_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t iface = USBDeluxe_DeviceDescriptor_InsertInterface(0, 2, HID_INTF, BOOT_INTF_SUBCLASS, hid_protocol);
	uint16_t dsc_off0 = usb_device_desc_ctx.raw.size;
	USBDeluxe_DeviceDescriptor_InsertHIDSpecific(0x0111, 0x00, 1, hid_report_desc_size);
	uint16_t dsc_off1 = usb_device_desc_ctx.raw.size;
	uint8_t ep = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN|USB_EP_DIR_OUT, _INTERRUPT, hid_report_size, 1, -1, -1);

	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_HID);

	USBDeluxeDevice_HID_Create(ctx->drv_ctx, userp, iface, ep, hid_report_size, hid_report_desc_size, dsc_off0, dsc_off1 - dsc_off0, hid_ops);

	return last_idx;
}

#endif