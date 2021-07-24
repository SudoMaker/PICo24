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
#include "usb_deluxe_device_cdc_ecm.h"
#include "PICo24/Core/Delay.h"

extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM

void USBDeluxeDevice_CDC_ECM_Create(USBDeluxeDevice_CDCECMContext *cdc_ctx, void *userp, uint8_t usb_iface_comm, uint8_t usb_iface_data,
				    uint8_t usb_ep_comm, uint8_t usb_ep_data, USBDeluxeDevice_CDC_ECM_IOps *io_ops) {
	memset(cdc_ctx, 0, sizeof(USBDeluxeDevice_CDCECMContext));

	if (io_ops) {
		memcpy(&cdc_ctx->io_ops, io_ops, sizeof(USBDeluxeDevice_CDC_ECM_IOps));
	}

	cdc_ctx->userp = userp;

	cdc_ctx->USB_IFACE_COMM = usb_iface_comm;
	cdc_ctx->USB_IFACE_DATA = usb_iface_data;

	cdc_ctx->USB_EP_COMM = usb_ep_comm;
	cdc_ctx->USB_EP_DATA = usb_ep_data;
}

void USBDeluxeDevice_CDC_ECM_Init(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	USBEnableEndpoint(cdc_ctx->USB_EP_COMM,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	USBEnableEndpoint(cdc_ctx->USB_EP_DATA,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

	cdc_ctx->CDCDataInHandle = USBRxOnePacket(cdc_ctx->USB_EP_DATA,(uint8_t *)&cdc_ctx->rx_buf, USBDeluxe_CDC_PKT_SIZE);
	cdc_ctx->CDCDataOutHandle = NULL;

	static const USBDeluxeDevice_CDC_State_Notification notif_template = {
		.bmRequestType = 0xa1,
		.bNotification = 0x00,
		.wValue = 0x01,
		.wIndex = 2,
		.wLength = 0
	};

	memcpy(&cdc_ctx->cdc_notif, &notif_template, sizeof(USBDeluxeDevice_CDC_State_Notification));

	USBTxOnePacket(cdc_ctx->USB_EP_COMM, (uint8_t *)&cdc_ctx->cdc_notif, sizeof(USBDeluxeDevice_CDC_State_Notification)-8);

	Vector_Init(&cdc_ctx->ntb_buffer, 1);
}


void USBDeluxeDevice_CDC_ECM_CheckRequest(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	/*
	 * If request recipient is not an interface then return
	 */
	if (SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;

	/*
	 * If request type is not class-specific then return
	 */
	if (SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) return;

	/*
	 * Interface ID must match interface numbers associated with
	 * CDC class, else return
	 */
	if ((SetupPkt.bIntfID != cdc_ctx->USB_IFACE_COMM)&&
	    (SetupPkt.bIntfID != cdc_ctx->USB_IFACE_DATA)) return;

	printf("NCM Setup: req: 0x%02x, wVal: 0x%02x, wIdx: 0x%02x, wLen: 0x%02x\n", SetupPkt.bRequest, SetupPkt.wValue, SetupPkt.wIndex, SetupPkt.wLength);
//
//	switch (SetupPkt.wIndex) {
//		case 0x02:
//			USBTxOnePacket(cdc_ctx->USB_EP_COMM, (uint8_t *)"\x01\x00\x00\x00\x00\x00\x00\x00", 8);
//			break;
//	}

	static const uint8_t ntb_params[] = {
		0x1c, 0x00,			// wLength
		0x01, 0x00,			// bmNtbFormatsSupported
		0x00, 0x06, 0x00, 0x00,		// dwNtbInMaxSize
		0x04, 0x00,			// wNdpInDivisor
		0x00, 0x00,			// wNdpInPayloadRemainder
		0x04, 0x00,			// wNdpInAlignment
		0x00, 0x00,			// reserved
		0x00, 0x06, 0x00, 0x00,		// dwNtbOutMaxSize
		0x04, 0x00,			// wNdpOutDivisor
		0x00, 0x00,			// wNdpOutPayloadRemainder
		0x04, 0x00,			// wNdpOutAlignment
		0x00, 0x00			// wNtbOutMaxDatagrams
	};


	switch (SetupPkt.bRequest) {
		//****** These commands are required ******//
		case GET_NTB_PARAMETERS:
		USBEP0SendROMPtr(ntb_params, sizeof(ntb_params), USB_EP0_INCLUDE_ZERO);
			break;
		default:
			break;
	}//end switch(SetupPkt.bRequest)

}//end USBCheckCDCRequest

void USBDeluxeDevice_CDC_ECM_TryRx(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	if (cdc_ctx->rx_queue_pending) {
		if (xQueueSendToBack(cdc_ctx->rx_queue, &cdc_ctx->rx_queue_pending_idx, 0) == pdPASS) {
			cdc_ctx->rx_queue_pending = 0;
		}

		return;
	}

	if (!USBHandleBusy(cdc_ctx->CDCDataInHandle)) {
		uint8_t cur_buf_idx = cdc_ctx->rx_buf_idx;
		uint8_t last_rx_len = USBHandleGetLength(cdc_ctx->CDCDataInHandle);


		cdc_ctx->rx_buf_idx++;
		cdc_ctx->rx_buf_idx %= sizeof(cdc_ctx->rx_buf_len);

		cdc_ctx->CDCDataInHandle = USBRxOnePacket(cdc_ctx->USB_EP_DATA, cdc_ctx->rx_buf[cdc_ctx->rx_buf_idx], USBDeluxe_CDC_PKT_SIZE);

		if (last_rx_len) {
//			printf("tryrx cur_buf_idx: %u, len: %u\n", cur_buf_idx, last_rx_len);
			cdc_ctx->rx_buf_len[cur_buf_idx] = last_rx_len;
			cdc_ctx->rx_buf_pos[cur_buf_idx] = 0;

			if (cdc_ctx->io_ops.RxDone) {
				cdc_ctx->io_ops.RxDone(cdc_ctx->userp, cdc_ctx->rx_buf[cur_buf_idx], last_rx_len);
			} else {
				if (xQueueSendToBack(cdc_ctx->rx_queue, &cur_buf_idx, 0) == pdPASS) {
					cdc_ctx->rx_queue_pending = 0;
				} else {
					cdc_ctx->rx_queue_pending = 1;
					cdc_ctx->rx_queue_pending_idx = cur_buf_idx;
				}
			}
		}
	}
}

uint8_t USBDeluxeDevice_CDC_ECM_DoTx(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	uint8_t tx_done = 0;

	USBMaskInterrupts();

	if (!USBHandleBusy(cdc_ctx->CDCDataOutHandle)) { // Last TX completed
		uint8_t cur_buf_idx = cdc_ctx->tx_buf_idx; // Last configured buffer index

		if (cdc_ctx->tx_buf_len[cur_buf_idx]) {
			cdc_ctx->CDCDataOutHandle = USBTxOnePacket(cdc_ctx->USB_EP_DATA, cdc_ctx->tx_buf[cur_buf_idx], cdc_ctx->tx_buf_len[cur_buf_idx]);

			cdc_ctx->tx_buf_idx++; // Switch to another buffer
			cdc_ctx->tx_buf_idx %= sizeof(cdc_ctx->tx_buf_len);
			cdc_ctx->tx_buf_len[cdc_ctx->tx_buf_idx] = 0; // Mark the 'another' buffer will be used as empty
			tx_done = 1;
		}
	} else {
		tx_done = 2;
	}

	USBUnmaskInterrupts();

	if (tx_done == 1) {
		xSemaphoreGive(cdc_ctx->tx_lock);
	}

	return tx_done;
}

void USBDeluxeDevice_CDC_ECM_Tasks(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	USBDeluxeDevice_CDC_ECM_DoTx(cdc_ctx);
	USBDeluxeDevice_CDC_ECM_TryRx(cdc_ctx);
}

int USBDeluxeDevice_CDC_ECM_AcquireRxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
	uint8_t idx;

	if (xQueuePeek(cdc_ctx->rx_queue, &idx, UINT16_MAX) == pdPASS) {
		user_buf->buf = cdc_ctx->rx_buf[idx];
		user_buf->buf_len = &cdc_ctx->rx_buf_len[idx];
		user_buf->buf_pos = &cdc_ctx->rx_buf_pos[idx];
		return 0;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

void USBDeluxeDevice_CDC_ECM_AdvanceRxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx) {
	uint8_t idx;
	xQueueReceive(cdc_ctx->rx_queue, &idx, UINT16_MAX);
}

int USBDeluxeDevice_CDC_ECM_AcquireTxBuffer(USBDeluxeDevice_CDCECMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
	if (xSemaphoreTake(cdc_ctx->tx_lock, UINT16_MAX)) {
		user_buf->buf = cdc_ctx->tx_buf[cdc_ctx->tx_buf_idx];
		user_buf->buf_len = &cdc_ctx->tx_buf_len[cdc_ctx->tx_buf_idx];
		return 0;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

ssize_t USBDeluxeDevice_CDC_ECM_ReadRaw(USBDeluxeDevice_CDCECMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;


	if (USBDeluxeDevice_CDC_ECM_AcquireRxBuffer(cdc_ctx, &user_buf)) {
		return -1;
	} else {
		uint8_t left = *user_buf.buf_len - *user_buf.buf_pos;

		if (len > left) {
			len = left;
		}

		memcpy(buf, user_buf.buf + *user_buf.buf_pos, len);
		*user_buf.buf_pos += len;

		if (*user_buf.buf_pos == *user_buf.buf_len) {
			USBDeluxeDevice_CDC_ECM_AdvanceRxBuffer(cdc_ctx);
		}

		return len;
	}
}

ssize_t USBDeluxeDevice_CDC_ECM_Read(USBDeluxeDevice_CDCECMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	while (USBDeluxeDevice_CDC_ECM_AcquireRxBuffer(cdc_ctx, &user_buf) != 0);

}

ssize_t USBDeluxeDevice_CDC_ECM_Write(USBDeluxeDevice_CDCECMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	if (USBDeluxeDevice_CDC_ECM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
		return -1;
	}

	if (len > USBDeluxe_CDC_BUF_SIZE) {
		len = USBDeluxe_CDC_BUF_SIZE;
	}

	memcpy(user_buf.buf, buf, len);
	*user_buf.buf_len = len;

	return len;
}

void USBDeluxe_DeviceDescriptor_InsertCDCECMSpecific(uint8_t comm_iface, uint8_t data_iface, uint8_t macaddr_str_idx) {
	/* CDC ECM Class-Specific Descriptors */

	// 5 bytes: Header Functional Descriptor
	uint8_t buf0[] = {
		0x5,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		DSC_FN_HEADER,				// bDescriptorSubtype (header functional descriptor)
		0x20,					// bcdCDC (CDC spec version this fw complies with: v1.20 [stored in little endian])
		0x01					// ^
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

	// 13 bytes: Ethernet Networking Functional Descriptor
	uint8_t buf1[] = {
		13,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		0x0f,					// bDescriptorSubtype (Ethernet Networking)
		macaddr_str_idx,			// iMACAddress
		0, 0, 0, 0,				// bmEthernetStatistics
		0xea, 0x05,				// wMaxSegmentSize
		0x00, 0x00,				// wMCFilters
		0x00,					// bNumberPowerFilters
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf1, sizeof(buf1));

	// 5 bytes: Union Functional Descriptor
	uint8_t buf2[] = {
		0x5,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		DSC_FN_UNION,				// bDescriptorSubtype (union functional)
		comm_iface,				// bControlInterface: Interface number of the communication class interface (1)
		data_iface,				// bSubordinateInterface0: Data class interface #2 is subordinate to this interface
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf2, sizeof(buf2));

	// 6 bytes: NCM Functional Descriptor
	uint8_t buf3[] = {
		6,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		0x1a,					// bDescriptorSubtype (NCM functional)
		0x00, 0x01,				// bcdNCM
		0,					// bmNetworkCapabilities
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf3, sizeof(buf3));
}

uint8_t USBDeluxe_DeviceFunction_Add_CDC_ECM(void *userp, const char *mac_addr, USBDeluxeDevice_CDC_ECM_IOps *io_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t mac_str_idx = USBDeluxe_DeviceDescriptor_AddString(mac_addr, -1);

	USBDeluxe_DeviceDescriptor_InsertIAD(usb_device_desc_ctx.used_interfaces, 2, 0x02, 0x0d, 0x00);

	uint8_t iface_comm = USBDeluxe_DeviceDescriptor_InsertInterface(0, 1, 0x02, 0x0d, 0x00);
	USBDeluxe_DeviceDescriptor_InsertCDCECMSpecific(iface_comm, usb_device_desc_ctx.used_interfaces, mac_str_idx);
	uint8_t ep_comm = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN, _INTERRUPT, 8, 2, -1, -1);

	USBDeluxe_DeviceDescriptor_InsertInterface(0, 0, DATA_INTF, 0, 1);
	usb_device_desc_ctx.used_interfaces--;
	uint8_t iface_data = USBDeluxe_DeviceDescriptor_InsertInterface(1, 2, DATA_INTF, 0, 1);
	uint8_t ep_data = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN|USB_EP_DIR_OUT, _BULK, 64, 0, -1, -1);

	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_CDC_ECM);

	USBDeluxeDevice_CDC_ECM_Create(ctx->drv_ctx, userp, iface_comm, iface_data, ep_comm, ep_data, io_ops);

	USBDeluxe_Device_TaskCreate(last_idx, "CDC_ECM");

	return last_idx;
}

#endif
