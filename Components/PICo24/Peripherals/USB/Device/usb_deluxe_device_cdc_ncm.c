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
#include "usb_deluxe_device_cdc_ncm.h"
#include "PICo24/Core/Delay.h"
#include "PICo24/Library/DebugTools.h"

extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM

void USBDeluxeDevice_CDC_NCM_Create(USBDeluxeDevice_CDCNCMContext *cdc_ctx, void *userp, uint8_t usb_iface_comm, uint8_t usb_iface_data,
				    uint8_t usb_ep_comm, uint8_t usb_ep_data, USBDeluxeDevice_CDC_NCM_IOps *io_ops) {
	memset(cdc_ctx, 0, sizeof(USBDeluxeDevice_CDCNCMContext));

	if (io_ops) {
		memcpy(&cdc_ctx->io_ops, io_ops, sizeof(USBDeluxeDevice_CDC_NCM_IOps));
	}

	cdc_ctx->userp = userp;

	cdc_ctx->USB_IFACE_COMM = usb_iface_comm;
	cdc_ctx->USB_IFACE_DATA = usb_iface_data;

	cdc_ctx->USB_EP_COMM = usb_ep_comm;
	cdc_ctx->USB_EP_DATA = usb_ep_data;

	cdc_ctx->rx_queue = xQueueCreate(1, 1);
	cdc_ctx->tx_lock = xSemaphoreCreateMutex();

	Vector_Init(&cdc_ctx->ntb_context.buffer, 1);
}

void USBDeluxeDevice_CDC_NCM_Init(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {
	USBEnableEndpoint(cdc_ctx->USB_EP_COMM,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
	USBEnableEndpoint(cdc_ctx->USB_EP_DATA,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

	cdc_ctx->CDCDataInHandle = USBRxOnePacket(cdc_ctx->USB_EP_DATA,(uint8_t *)&cdc_ctx->rx_buf, USBDeluxe_CDC_PKT_SIZE);
	cdc_ctx->CDCDataOutHandle = NULL;

	cdc_ctx->notif_sent = 1;
}


void USBDeluxeDevice_CDC_NCM_CheckRequest(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {
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
		0x00, 0x08, 0x00, 0x00,		// dwNtbInMaxSize
		0x04, 0x00,			// wNdpInDivisor
		0x00, 0x00,			// wNdpInPayloadRemainder
		0x04, 0x00,			// wNdpInAlignment
		0x00, 0x00,			// reserved
		0x00, 0x08, 0x00, 0x00,		// dwNtbOutMaxSize
		0x04, 0x00,			// wNdpOutDivisor
		0x00, 0x00,			// wNdpOutPayloadRemainder
		0x04, 0x00,			// wNdpOutAlignment
		0x10, 0x00			// wNtbOutMaxDatagrams
	};


	switch (SetupPkt.bRequest) {
		//****** These commands are required ******//
		case GET_NTB_PARAMETERS:
		USBEP0SendROMPtr(ntb_params, sizeof(ntb_params), USB_EP0_INCLUDE_ZERO);
			break;
		case SET_ETHERNET_PACKET_FILTER:
			inPipes[0].info.bits.busy = 1;
			break;
		default:
			break;
	}//end switch(SetupPkt.bRequest)

	cdc_ctx->notif_sent = 0;

}//end USBCheckCDCRequest

void USBDeluxeDevice_CDC_NCM_TryRx(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {

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

uint8_t USBDeluxeDevice_CDC_NCM_DoTx(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {
	uint8_t tx_done = 0;

	taskENTER_CRITICAL();
	USBMaskInterrupts();

	if (!USBHandleBusy(cdc_ctx->CDCDataOutHandle)) { // Last TX completed
		uint8_t cur_buf_idx = cdc_ctx->tx_buf_idx; // Last configured buffer index

		if (cdc_ctx->tx_buf_len[cur_buf_idx] || cdc_ctx->tx_buf_has_data[cur_buf_idx]) {
			printf("NCM DoTx: %u\n", cdc_ctx->tx_buf_len[cur_buf_idx]);
			cdc_ctx->CDCDataOutHandle = USBTxOnePacket(cdc_ctx->USB_EP_DATA, cdc_ctx->tx_buf[cur_buf_idx], cdc_ctx->tx_buf_len[cur_buf_idx]);

			cdc_ctx->tx_buf_idx++; // Switch to another buffer
			cdc_ctx->tx_buf_idx %= sizeof(cdc_ctx->tx_buf_len);
			cdc_ctx->tx_buf_len[cdc_ctx->tx_buf_idx] = 0; // Mark the 'another' buffer will be used as empty
			cdc_ctx->tx_buf_has_data[cdc_ctx->tx_buf_idx] = 0;
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

void USBDeluxeDevice_CDC_NCM_Tasks(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {
	if (!cdc_ctx->notif_sent) {
		static const USBDeluxeDevice_CDC_State_Notification notif_template = {
			.bmRequestType = 0xa1,
			.bNotification = 0x00,
			.wValue = 0x01,
			.wIndex = 2,
			.wLength = 0
		};

		static const uint8_t notif[] = {
			0xa1, 0x2a,   0x01, 0x00,   0x02, 0x00,   0x08, 0x00,
			0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
		};

		memcpy(&cdc_ctx->cdc_notif, &notif, sizeof(notif));
		USBMaskInterrupts();
		cdc_ctx->CDCCommOutHandle = USBTxOnePacket(cdc_ctx->USB_EP_COMM, (uint8_t *)&cdc_ctx->cdc_notif, sizeof(notif));
		USBUnmaskInterrupts();

		while (USBHandleBusy(cdc_ctx->CDCCommOutHandle));

		memcpy(&cdc_ctx->cdc_notif, &notif_template, sizeof(USBDeluxeDevice_CDC_State_Notification));

		USBMaskInterrupts();
		cdc_ctx->CDCCommOutHandle = USBTxOnePacket(cdc_ctx->USB_EP_COMM, (uint8_t *)&cdc_ctx->cdc_notif, sizeof(USBDeluxeDevice_CDC_State_Notification));
		USBUnmaskInterrupts();

		cdc_ctx->notif_sent = 1;
	}

	USBDeluxeDevice_CDC_NCM_DoTx(cdc_ctx);
	USBDeluxeDevice_CDC_NCM_TryRx(cdc_ctx);
}

int USBDeluxeDevice_CDC_NCM_AcquireRxBuffer(USBDeluxeDevice_CDCNCMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
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

void USBDeluxeDevice_CDC_NCM_AdvanceRxBuffer(USBDeluxeDevice_CDCNCMContext *cdc_ctx) {
	uint8_t idx;
	xQueueReceive(cdc_ctx->rx_queue, &idx, UINT16_MAX);
}

int USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(USBDeluxeDevice_CDCNCMContext *cdc_ctx, USBDeluxeDevice_CDC_UserBuffer *user_buf) {
	if (xSemaphoreTake(cdc_ctx->tx_lock, UINT16_MAX)) {
		user_buf->buf = cdc_ctx->tx_buf[cdc_ctx->tx_buf_idx];
		user_buf->buf_len = &cdc_ctx->tx_buf_len[cdc_ctx->tx_buf_idx];
		user_buf->buf_pos = &cdc_ctx->tx_buf_has_data[cdc_ctx->tx_buf_idx];
		return 0;
	} else {
		errno = EAGAIN;
		return -1;
	}
}

ssize_t USBDeluxeDevice_CDC_NCM_ReadRaw(USBDeluxeDevice_CDCNCMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	if (USBDeluxeDevice_CDC_NCM_AcquireRxBuffer(cdc_ctx, &user_buf)) {
		return -1;
	} else {
		uint8_t left = *user_buf.buf_len - *user_buf.buf_pos;

		if (len > left) {
			len = left;
		}

		memcpy(buf, user_buf.buf + *user_buf.buf_pos, len);
		*user_buf.buf_pos += len;

		if (*user_buf.buf_pos == *user_buf.buf_len) {
			USBDeluxeDevice_CDC_NCM_AdvanceRxBuffer(cdc_ctx);
		}

		return len;
	}
}

ssize_t USBDeluxeDevice_CDC_NCM_Read(USBDeluxeDevice_CDCNCMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	// Step 1: Ensure a whole NTB is received
	if (Vector_Empty(&cdc_ctx->ntb_context.buffer)) {
		while (1) {
			if (USBDeluxeDevice_CDC_NCM_AcquireRxBuffer(cdc_ctx, &user_buf) == 0) {
				USBDeluxeDevice_CDC_UserBuffer user_buf2;
				while (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf2) != 0);
				memcpy(user_buf2.buf, user_buf.buf, *user_buf.buf_len);
				*user_buf2.buf_len = *user_buf.buf_len;
				*user_buf2.buf_pos = 1;

				uint8_t this_pkt_len = *user_buf.buf_len;
				Vector_PushBack2(&cdc_ctx->ntb_context.buffer, user_buf.buf, this_pkt_len);
				USBDeluxeDevice_CDC_NCM_AdvanceRxBuffer(cdc_ctx);

				printf("NCM_Read: got raw packet, len = %u\n", this_pkt_len);

				// If the pkt's len < max USB packet len, then it's the last piece of NTB
				if (this_pkt_len < USBDeluxe_CDC_PKT_SIZE) {
					cdc_ctx->ntb_context.ptr_upper_limit = Vector_At(&cdc_ctx->ntb_context.buffer, cdc_ctx->ntb_context.buffer.size);
					printf("NCM_Read: packet read done, total len = %u, ptr end = 0x%p\n", cdc_ctx->ntb_context.buffer.size, cdc_ctx->ntb_context.ptr_upper_limit);

					hexdump(cdc_ctx->ntb_context.buffer.data, cdc_ctx->ntb_context.buffer.size);
					break;
				}
			}
		}
	}

	ssize_t ret = -1;

	uint8_t *ntb_data = cdc_ctx->ntb_context.buffer.data;
	size_t ntb_len = cdc_ctx->ntb_context.buffer.size;

	// Step 2: Decode header if not already done
	if (!cdc_ctx->ntb_context.header.len) {
		if (ntb_len > 12 && memcmp(ntb_data, "NCMH\x0c\x00", 6) == 0) {
			memcpy(&cdc_ctx->ntb_context.header.seq, ntb_data + 6, sizeof(uint16_t));
			memcpy(&cdc_ctx->ntb_context.header.len, ntb_data + 8, sizeof(uint16_t));
			memcpy(&cdc_ctx->ntb_context.header.first_ndp_offset, ntb_data + 10, sizeof(uint16_t));

			if (cdc_ctx->ntb_context.header.len != ntb_len) {
				printf("NCM_Read: ERROR: pkt size mismatch\n");
				goto free_ntb_and_return;
			}

			if (cdc_ctx->ntb_context.header.first_ndp_offset >= ntb_len) {
				printf("NCM_Read: ERROR: first_ndp_offset beyond EOF\n");
				goto free_ntb_and_return;
			}

			cdc_ctx->ntb_context.current_ndp_offset = cdc_ctx->ntb_context.header.first_ndp_offset;

			printf("NCM_Read: header.seq = %u\n", cdc_ctx->ntb_context.header.seq);
			printf("NCM_Read: header.len = %u\n", cdc_ctx->ntb_context.header.len);
			printf("NCM_Read: header.first_ndp_offset = %u\n", cdc_ctx->ntb_context.header.first_ndp_offset);

		} else {
			// Not a valid NTB header
			printf("NCM_Read: ERROR: NTB header not valid\n");
			hexdump(ntb_data, ntb_len);
			goto free_ntb_and_return;
		}
	}

	// Step 3: Parse NDPs
	while (cdc_ctx->ntb_context.current_ndp_offset) {
		if (cdc_ctx->ntb_context.current_ndp_datagram_ptr) {
			if ((uint8_t *)cdc_ctx->ntb_context.current_ndp_datagram_ptr > cdc_ctx->ntb_context.ptr_upper_limit) {
				printf("NCM_Read: ERROR: NDP entry beyond EOF\n");
				goto free_ntb_and_return;
			}

			uint16_t dgm_offset = cdc_ctx->ntb_context.current_ndp_datagram_ptr[0];
			uint16_t dgm_len = cdc_ctx->ntb_context.current_ndp_datagram_ptr[1];

			if (dgm_offset) {
				printf("NCM_Read: ndp.dgm_offset = %u\n", dgm_offset);

				uint8_t *dgm_abs_ptr = Vector_At(&cdc_ctx->ntb_context.buffer, dgm_offset);

				if (dgm_abs_ptr + dgm_len > cdc_ctx->ntb_context.ptr_upper_limit) {
					printf("NCM_Read: ERROR: dgm entry beyond EOF\n");
					goto free_ntb_and_return;
				}

				size_t copy_len = len > dgm_len ? dgm_len : len;
				memcpy(buf, dgm_abs_ptr, copy_len);
				cdc_ctx->ntb_context.current_ndp_datagram_ptr += 2; // elem size is 2
				return copy_len;
			} else {
				cdc_ctx->ntb_context.current_ndp_offset = cdc_ctx->ntb_context.next_ndp_offset;
			}
		} else {
			uint8_t *ndp_data = ntb_data + cdc_ctx->ntb_context.current_ndp_offset;
			uint16_t ndp_len;

			if (memcmp("NCM0", ndp_data, 4) != 0) {
				printf("NCM_Read: ERROR: NDP header is not NCM0\n");
				goto free_ntb_and_return;
			}

			memcpy(&ndp_len, ndp_data + 4, sizeof(uint16_t));
			memcpy(&cdc_ctx->ntb_context.next_ndp_offset, ndp_data + 6, sizeof(uint16_t));

			if (cdc_ctx->ntb_context.next_ndp_offset >= ntb_len) {
				printf("NCM_Read: ERROR: next_ndp_offset beyond EOF\n");
				goto free_ntb_and_return;
			}

			cdc_ctx->ntb_context.current_ndp_datagram_ptr = (uint16_t *) (ndp_data + 8);
		}
	}

free_ntb_and_return:
	Vector_Clear(&cdc_ctx->ntb_context.buffer);
	memset(&cdc_ctx->ntb_context, 0, sizeof(cdc_ctx->ntb_context));
	Vector_Init(&cdc_ctx->ntb_context.buffer, 1);
	printf("NCM_Read: reset\n");

	return ret;
}

ssize_t USBDeluxeDevice_CDC_NCM_ReadOS(USBDeluxeDevice_CDCNCMContext *cdc_ctx, uint8_t *buf, size_t len) {
	USBDeluxeDevice_CDC_UserBuffer user_buf;

	while (1) {
		if (cdc_ctx->ntb_context.datagram_index) {

			if (!cdc_ctx->ntb_context.datagram_offset) {
				uint8_t *ndp = Vector_At(&cdc_ctx->ntb_context.buffer, cdc_ctx->ntb_context.current_ndp_offset);
				memcpy(&cdc_ctx->ntb_context.datagram_offset, ndp + 4 + (4 * cdc_ctx->ntb_context.datagram_index), sizeof(uint16_t));
				memcpy(&cdc_ctx->ntb_context.datagram_length, ndp + 6 + (4 * cdc_ctx->ntb_context.datagram_index), sizeof(uint16_t));

				printf("!!! DGM OFFSET: %02x\n", cdc_ctx->ntb_context.datagram_offset);
			}

			if (cdc_ctx->ntb_context.datagram_offset == 0 && cdc_ctx->ntb_context.datagram_length == 0) {
				cdc_ctx->ntb_context.datagram_index = 0;
				cdc_ctx->ntb_context.current_ndp_length = 0;
				cdc_ctx->ntb_context.current_ndp_offset = cdc_ctx->ntb_context.next_ndp_offset;
			} else if (cdc_ctx->ntb_context.buffer.size >= (cdc_ctx->ntb_context.datagram_offset + cdc_ctx->ntb_context.datagram_length)) {
				uint8_t *datagram = Vector_At(&cdc_ctx->ntb_context.buffer, cdc_ctx->ntb_context.datagram_offset);
				size_t copy_len = len;
				if (cdc_ctx->ntb_context.datagram_length < len) {
					copy_len = cdc_ctx->ntb_context.datagram_length;
				}

				memcpy(buf, datagram, copy_len);

				cdc_ctx->ntb_context.datagram_index += 1;
				cdc_ctx->ntb_context.datagram_offset = 0;

				return copy_len;
			}
		} else {
			uint8_t *ndp = Vector_At(&cdc_ctx->ntb_context.buffer, cdc_ctx->ntb_context.current_ndp_offset);
			if (cdc_ctx->ntb_context.current_ndp_length) {
				printf("!!! NDP LENGTH %x\n", cdc_ctx->ntb_context.current_ndp_length);
				if (cdc_ctx->ntb_context.buffer.size >= (cdc_ctx->ntb_context.current_ndp_offset + cdc_ctx->ntb_context.current_ndp_length)) {
					memcpy(&cdc_ctx->ntb_context.next_ndp_offset, ndp + 6, sizeof(uint16_t));
					cdc_ctx->ntb_context.datagram_index = 1;
					continue;
				}
			} else {
				printf("!!! BUF SIZE %02x, NDP OFFSET %02x\n", cdc_ctx->ntb_context.buffer.size, cdc_ctx->ntb_context.current_ndp_offset + 6);
				if (cdc_ctx->ntb_context.buffer.size > (cdc_ctx->ntb_context.current_ndp_offset + 6)) {
					if (memcmp(ndp, "NCM", 3) == 0 && (ndp[3] == '0' || ndp[3] == '1')) {
						memcpy(&cdc_ctx->ntb_context.current_ndp_length, ndp + 4, sizeof(uint16_t));
						continue;
					}
				}
			}
		}

		if (
			cdc_ctx->ntb_context.header_total_length
			&& (cdc_ctx->ntb_context.buffer.size >= cdc_ctx->ntb_context.header_total_length)
			) {
			// Clean up
			printf("!!! CLEANUP %02x, %02x\n", cdc_ctx->ntb_context.buffer.size, cdc_ctx->ntb_context.header_total_length);
			Vector_Clear(&cdc_ctx->ntb_context.buffer);
			memset(&cdc_ctx->ntb_context, 0, sizeof(cdc_ctx->ntb_context));
			Vector_Init(&cdc_ctx->ntb_context.buffer, 1);
		}

		if (USBDeluxeDevice_CDC_NCM_AcquireRxBuffer(cdc_ctx, &user_buf) != 0) {
			continue;
		}

		uint8_t raw_data_len = *user_buf.buf_len;
		uint8_t *raw_data = user_buf.buf;

		if (!cdc_ctx->ntb_context.header_total_length) {
			if (raw_data_len >= 12 && memcmp(raw_data, "NCMH\x0c\x00", 6) == 0) {
				memcpy(&cdc_ctx->ntb_context.header_seq, raw_data + 6, sizeof(uint16_t));
				memcpy(&cdc_ctx->ntb_context.header_total_length, raw_data + 8, sizeof(uint16_t));
				memcpy(&cdc_ctx->ntb_context.current_ndp_offset, raw_data + 10, sizeof(uint16_t));

				printf("!!! NDP OFFSET: %02x\n", cdc_ctx->ntb_context.current_ndp_offset);

			} else {
				printf("!!! NDP Dropped\n");
				USBDeluxeDevice_CDC_NCM_AdvanceRxBuffer(cdc_ctx);
				continue;
			}
		}

		Vector_PushBack2(&cdc_ctx->ntb_context.buffer, raw_data, raw_data_len);
		USBDeluxeDevice_CDC_NCM_AdvanceRxBuffer(cdc_ctx);
	}
}

ssize_t USBDeluxeDevice_CDC_NCM_Write(USBDeluxeDevice_CDCNCMContext *cdc_ctx, uint8_t *buf, size_t len) {
	static const uint8_t ntb_template[0x1c] = {
		// Header
		'N', 'C', 'M', 'H',	// Offset 0x00: NTH
		0x0c, 0x00,		// Offset 0x04: NTH len
		0x00, 0x00,		// Offset 0x06: seq *
		0x00, 0x00,		// Offset 0x08: len *
		0x0c, 0x00,		// Offset 0x0a: NDP offset
		// NDP
		'N', 'C', 'M', '0',	// Offset 0x0c: NDP header
		0x10, 0x00,		// Offset 0x10: NDP len
		0x00, 0x00,		// Offset 0x12: Next NDP offset
		0xb8, 0x00,		// Offset 0x14: DGRAM offset [0], 0xb8
		0x00, 0x00,		// Offset 0x16: DGRAM len [0] *
		0x00, 0x00,		// Offset 0x18: DGRAM ending
		0x00, 0x00,		// Offset 0x1a: DGRAM ending
	};

	USBDeluxeDevice_CDC_UserBuffer user_buf;

	// Packet #0
	if (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
		return -1;
	}

	memcpy(user_buf.buf, ntb_template, sizeof(ntb_template));
	memcpy(user_buf.buf+0x06, &cdc_ctx->ntb_tx_seq, sizeof(uint16_t));
	uint16_t total_len = len + 0xb8;
	memcpy(user_buf.buf+0x08, &total_len, sizeof(uint16_t));
	memcpy(user_buf.buf+0x16, &len, sizeof(uint16_t));
	*user_buf.buf_len = USBDeluxe_CDC_PKT_SIZE;

	// Packet #1
	if (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
		return -1;
	}
	*user_buf.buf_len = USBDeluxe_CDC_PKT_SIZE;

	// Packet #2
	if (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
		return -1;
	}

	uint8_t first_copy_len = 0xc0 - 0xb8;
	uint8_t garbage_len = USBDeluxe_CDC_PKT_SIZE - first_copy_len;
	uint8_t copy_len = len > first_copy_len ? first_copy_len : len;

	memcpy(user_buf.buf + garbage_len, 0, copy_len);

	*user_buf.buf_len = garbage_len + copy_len;

	printf("first_copy_len: %u\n", copy_len);

	size_t processed_len = copy_len;

	while (processed_len < len) {
		if (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
			return processed_len;
		}

		size_t left_len = len - processed_len;
		copy_len = left_len > USBDeluxe_CDC_PKT_SIZE ? USBDeluxe_CDC_PKT_SIZE : left_len;

		memcpy(user_buf.buf, buf+processed_len, copy_len);
		*user_buf.buf_len = copy_len;

		if (copy_len != USBDeluxe_CDC_PKT_SIZE) {
			uint8_t mod_remaining = copy_len % 8;
			*user_buf.buf_len += (8 - mod_remaining);
			printf("mod_remaining: %u\n", mod_remaining);

		}

		processed_len += copy_len;

		printf("copy_len: %u\n", copy_len);
	}

	if (copy_len == USBDeluxe_CDC_PKT_SIZE) {
		if (USBDeluxeDevice_CDC_NCM_AcquireTxBuffer(cdc_ctx, &user_buf) == -1) {
			return processed_len;
		}

		*user_buf.buf_pos = 1;
		printf("Send ZLP\n");
	}

	cdc_ctx->ntb_tx_seq++;

	return processed_len;
}

void USBDeluxe_DeviceDescriptor_InsertCDCNCMSpecific(uint8_t comm_iface, uint8_t data_iface, uint8_t macaddr_str_idx) {
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

uint8_t USBDeluxe_DeviceFunction_Add_CDC_NCM(void *userp, const char *mac_addr, USBDeluxeDevice_CDC_NCM_IOps *io_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t mac_str_idx = USBDeluxe_DeviceDescriptor_AddString(mac_addr, -1);

	USBDeluxe_DeviceDescriptor_InsertIAD(usb_device_desc_ctx.used_interfaces, 2, 0x02, 0x0d, 0x00);

	uint8_t iface_comm = USBDeluxe_DeviceDescriptor_InsertInterface(0, 1, 0x02, 0x0d, 0x00);
	USBDeluxe_DeviceDescriptor_InsertCDCNCMSpecific(iface_comm, usb_device_desc_ctx.used_interfaces, mac_str_idx);
	uint8_t ep_comm = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN, _INTERRUPT, 24, 2, -1, -1);

	USBDeluxe_DeviceDescriptor_InsertInterface(0, 0, DATA_INTF, 0, 1);
	usb_device_desc_ctx.used_interfaces--;
	uint8_t iface_data = USBDeluxe_DeviceDescriptor_InsertInterface(1, 2, DATA_INTF, 0, 1);
	uint8_t ep_data = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN|USB_EP_DIR_OUT, _BULK, 64, 0, -1, -1);

	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_CDC_NCM);

	USBDeluxeDevice_CDC_NCM_Create(ctx->drv_ctx, userp, iface_comm, iface_data, ep_comm, ep_data, io_ops);

	USBDeluxe_Device_TaskCreate(last_idx, "CDC_NCM");

	return last_idx;
}

#endif
