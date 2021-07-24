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
#include "usb_deluxe_device_cdc_acm.h"


extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM


void USBDeluxeDevice_CDC_ACM_Create(USBDeluxeDevice_CDCACMContext *cdc_ctx, void *userp, uint8_t usb_iface_comm, uint8_t usb_iface_data,
				    uint8_t usb_ep_comm, uint8_t usb_ep_data, USBDeluxeDevice_CDC_ACM_IOps *io_ops) {
	memset(cdc_ctx, 0, sizeof(USBDeluxeDevice_CDCACMContext));

	if (io_ops) {
		memcpy(&cdc_ctx->io_ops, io_ops, sizeof(USBDeluxeDevice_CDC_ACM_IOps));
	}

	cdc_ctx->userp = userp;

	cdc_ctx->USB_IFACE_COMM = usb_iface_comm;
	cdc_ctx->USB_IFACE_DATA = usb_iface_data;

	cdc_ctx->USB_EP_COMM = usb_ep_comm;
	cdc_ctx->USB_EP_DATA = usb_ep_data;

	cdc_ctx->rx_queue = xQueueCreate(1, 1);
	cdc_ctx->tx_lock = xSemaphoreCreateMutex();
}

void USBDeluxeDevice_CDC_ACM_Init(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
	// Abstract line coding information
	cdc_ctx->line_coding.dwDTERate = 2000000;	// baud rate
	cdc_ctx->line_coding.bCharFormat = 0x00;	// 1 stop bit
	cdc_ctx->line_coding.bParityType = 0x00;	// None
	cdc_ctx->line_coding.bDataBits = 0x08;		// 5,6,7,8, or 16

//	USBEnableEndpoint(cdc_ctx->USB_EP_COMM,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	USBEnableEndpoint(cdc_ctx->USB_EP_DATA,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

	cdc_ctx->CDCDataInHandle = USBRxOnePacket(cdc_ctx->USB_EP_DATA,(uint8_t *)cdc_ctx->rx_buf, USBDeluxe_CDC_PKT_SIZE);
	cdc_ctx->CDCDataOutHandle = NULL;

}

void USBDeluxeDevice_CDC_ACM_EP0RxHandler_LineCoding(void *userp) {
	USBDeluxeDevice_CDCACMContext *cdc_ctx = userp;

	if (cdc_ctx->io_ops.SetLineCoding) {
		cdc_ctx->io_ops.SetLineCoding(cdc_ctx->userp, &cdc_ctx->line_coding);
	}
}

void USBDeluxeDevice_CDC_ACM_EP0RxHandler_ControlLineState(void *userp) {
	USBDeluxeDevice_CDCACMContext *cdc_ctx = userp;

	if (cdc_ctx->io_ops.SetControlLineState) {
		cdc_ctx->io_ops.SetControlLineState(cdc_ctx->userp, &cdc_ctx->control_signal_bitmap);
	}
}

void USBDeluxeDevice_CDC_ACM_CheckRequest(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
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

	static const uint8_t dummy[8];

	switch (SetupPkt.bRequest) {
		//****** These commands are required ******//
		case SEND_ENCAPSULATED_COMMAND:
		case GET_ENCAPSULATED_RESPONSE:
		USBEP0SendROMPtr(dummy, sizeof(dummy), USB_EP0_INCLUDE_ZERO);
			break;
			//****** End of required commands ******//


		case SET_LINE_CODING:
			outPipes[0].pFuncUserP = cdc_ctx;
			USBEP0Receive((uint8_t * volatile)&cdc_ctx->line_coding, SetupPkt.wLength, USBDeluxeDevice_CDC_ACM_EP0RxHandler_LineCoding);
			break;

		case GET_LINE_CODING:
		USBEP0SendRAMPtr((uint8_t*)&cdc_ctx->line_coding, LINE_CODING_LENGTH, USB_EP0_INCLUDE_ZERO);
			break;

		case SET_CONTROL_LINE_STATE:
			cdc_ctx->control_signal_bitmap._byte = (uint8_t)SetupPkt.wValue;

			if (cdc_ctx->io_ops.SetControlLineState) {
				cdc_ctx->io_ops.SetControlLineState(cdc_ctx->userp, &cdc_ctx->control_signal_bitmap);
			}

			//------------------------------------------------------------------
			//One way to control the RTS pin is to allow the USB host to decide the value
			//that should be output on the RTS pin.  Although RTS and CTS pin functions
			//are technically intended for UART hardware based flow control, some legacy
			//UART devices use the RTS pin like a "general purpose" output pin
			//from the PC host.  In this usage model, the RTS pin is not related
			//to flow control for RX/TX.
			//In this scenario, the USB host would want to be able to control the RTS
			//pin, and the below line of code should be uncommented.
			//However, if the intention is to implement true RTS/CTS flow control
			//for the RX/TX pair, then this application firmware should override
			//the USB host's setting for RTS, and instead generate a real RTS signal,
			//based on the amount of remaining buffer space available for the
			//actual hardware UART of this microcontroller.  In this case, the
			//below code should be left commented out, but instead RTS should be
			//controlled in the application firmware responsible for operating the
			//hardware UART of this microcontroller.
			//---------
			//CONFIGURE_RTS(control_signal_bitmap.CARRIER_CONTROL);
			//------------------------------------------------------------------

			inPipes[0].info.bits.busy = 1;
			break;
		default:
			break;
	}

}

void USBDeluxeDevice_CDC_ACM_TryRx(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
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

uint8_t USBDeluxeDevice_CDC_ACM_DoTx(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
	uint8_t tx_done = 0;

	taskENTER_CRITICAL();
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
	taskEXIT_CRITICAL();

	if (tx_done == 1) {
		xSemaphoreGive(cdc_ctx->tx_lock);
	}

	return tx_done;
}

void USBDeluxeDevice_CDC_ACM_Tasks(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
	USBDeluxeDevice_CDC_ACM_DoTx(cdc_ctx);
	USBDeluxeDevice_CDC_ACM_TryRx(cdc_ctx);
}

int USBDeluxeDevice_CDC_ACM_AcquireRxBuffer(USBDeluxeDevice_CDCACMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
	uint8_t idx;

	if (xQueuePeek(cdc_ctx->rx_queue, &idx, UINT16_MAX) == pdPASS) {
		user_buf->buf = cdc_ctx->rx_buf[idx];
		user_buf->buf_len = &cdc_ctx->rx_buf_len[idx];
		user_buf->buf_pos = &cdc_ctx->rx_buf_pos[idx];
//		printf("idx: %u\n", idx);
		return 0;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

void USBDeluxeDevice_CDC_ACM_AdvanceRxBuffer(USBDeluxeDevice_CDCACMContext *cdc_ctx) {
	uint8_t idx;
	xQueueReceive(cdc_ctx->rx_queue, &idx, UINT16_MAX);
}

int USBDeluxeDevice_CDC_ACM_AcquireTxBuffer(USBDeluxeDevice_CDCACMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
	if (xSemaphoreTake(cdc_ctx->tx_lock, UINT16_MAX)) {
		user_buf->buf = cdc_ctx->tx_buf[cdc_ctx->tx_buf_idx];
		user_buf->buf_len = &cdc_ctx->tx_buf_len[cdc_ctx->tx_buf_idx];
		return 0;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

ssize_t USBDeluxeDevice_CDC_ACM_Read(USBDeluxeDevice_CDCACMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;


	if (USBDeluxeDevice_CDC_ACM_AcquireRxBuffer(cdc_ctx, &user_buf)) {
		return -1;
	} else {
		uint8_t left = *user_buf.buf_len - *user_buf.buf_pos;
//		printf("+ len %u pos %u\n", *user_buf.buf_len, *user_buf.buf_pos);

		if (len > left) {
			len = left;
		}

		memcpy(buf, user_buf.buf + *user_buf.buf_pos, len);
		*user_buf.buf_pos += len;

//		printf("- len %u pos %u\n", *user_buf.buf_len, *user_buf.buf_pos);

		if (*user_buf.buf_pos == *user_buf.buf_len) {
//			printf("advance\n");
			USBDeluxeDevice_CDC_ACM_AdvanceRxBuffer(cdc_ctx);
		}

		return len;
	}
}

ssize_t USBDeluxeDevice_CDC_ACM_Write(USBDeluxeDevice_CDCACMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	if (USBDeluxeDevice_CDC_ACM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
		return -1;
	}

	if (len > USBDeluxe_CDC_BUF_SIZE) {
		len = USBDeluxe_CDC_BUF_SIZE;
	}

	memcpy(user_buf.buf, buf, len);
	*user_buf.buf_len = len;

	return len;
}

void USBDeluxe_DeviceDescriptor_InsertCDCACMSpecific(uint8_t comm_iface, uint8_t data_iface) {
	/* CDC Class-Specific Descriptors */

	// 5 bytes: Header Functional Descriptor
	uint8_t buf0[] = {
		0x5,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		DSC_FN_HEADER,				// bDescriptorSubtype (header functional descriptor)
		0x10,					// bcdCDC (CDC spec version this fw complies with: v1.20 [stored in little endian])
		0x01					// ^
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

	// 4 bytes: Abstract Control Management Functional Descriptor
	uint8_t buf1[] = {
		0x4,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		DSC_FN_ACM,				// bDescriptorSubtype (abstract control management)
		0x2,					// bmCapabilities: D1 (see PSTN120.pdf Table 4)
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

	// 5 bytes: Call Management Functional Descriptor
	uint8_t buf3[] = {
		0x5,					// Size of this descriptor in bytes
		CS_INTERFACE,				// bDescriptorType (class specific)
		DSC_FN_CALL_MGT,				// bDescriptorSubtype (call management functional)
		0x00,					// bmCapabilities: device doesn't handle call management
		data_iface,				// bDataInterface: Data class interface ID used for the optional call management
	};
	Vector_PushBack2(&usb_device_desc_ctx.raw, buf3, sizeof(buf3));

}

uint8_t USBDeluxe_DeviceFunction_Add_CDC_ACM(void *userp, USBDeluxeDevice_CDC_ACM_IOps *io_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	USBDeluxe_DeviceDescriptor_InsertIAD(usb_device_desc_ctx.used_interfaces, 2, COMM_INTF, ABSTRACT_CONTROL_MODEL, V25TER);

	uint8_t iface_comm = USBDeluxe_DeviceDescriptor_InsertInterface(0, 1, COMM_INTF, ABSTRACT_CONTROL_MODEL, V25TER);
	USBDeluxe_DeviceDescriptor_InsertCDCACMSpecific(iface_comm, usb_device_desc_ctx.used_interfaces);
	uint8_t ep_comm = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN, _INTERRUPT, 10, 2, -1, -1);

	uint8_t iface_data = USBDeluxe_DeviceDescriptor_InsertInterface(0, 2, DATA_INTF, 0, 0);
	uint8_t ep_data = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN|USB_EP_DIR_OUT, _BULK, 64, 0, -1, -1);

	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_CDC_ACM);

	USBDeluxeDevice_CDC_ACM_Create(ctx->drv_ctx, userp, iface_comm, iface_data, ep_comm, ep_data, io_ops);

	USBDeluxe_Device_TaskCreate(last_idx, "CDC_ACM");

	return last_idx;
}

#endif
