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
#include "usb_deluxe_device_midi.h"

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MIDI

void USBDeluxeDevice_MIDI_Create(USBDeluxeDevice_MIDIContext *midi_ctx, void *userp, uint8_t usb_iface_ac, uint8_t usb_iface_ms, uint8_t usb_ep_ms, USBDeluxeDevice_MIDI_Operations *midi_ops) {
	memset(midi_ctx, 0, sizeof(USBDeluxeDevice_MIDIContext));

	midi_ctx->userp = userp;
	midi_ctx->USB_IFACE_AC = usb_iface_ac;
	midi_ctx->USB_IFACE_MS = usb_iface_ms;
	midi_ctx->USB_EP_MS = usb_ep_ms;

	memcpy(&midi_ctx->ops, midi_ops, sizeof(USBDeluxeDevice_MIDI_Operations));
}

void USBDeluxeDevice_MIDI_Init(USBDeluxeDevice_MIDIContext *midi_ctx) {
	USBEnableEndpoint(midi_ctx->USB_EP_MS, USB_OUT_ENABLED | USB_IN_ENABLED | USB_HANDSHAKE_ENABLED | USB_DISALLOW_SETUP);
	midi_ctx->HandleRx = USBRxOnePacket(midi_ctx->USB_EP_MS, midi_ctx->buffer, sizeof(midi_ctx->buffer));
}

void USBDeluxeDevice_MIDI_Tasks(USBDeluxeDevice_MIDIContext *midi_ctx) {
	if (midi_ctx->HandleRx) {
		if (USBHandleBusy(midi_ctx->HandleRx)) {
			return;
		} else {
			uint8_t len = USBHandleGetLength(midi_ctx->HandleRx);
			if (midi_ctx->ops.RxDone) {
				midi_ctx->ops.RxDone(midi_ctx->userp, midi_ctx->buffer, len);
			}
		}
	}

	midi_ctx->HandleRx = USBRxOnePacket(midi_ctx->USB_EP_MS, midi_ctx->buffer, sizeof(midi_ctx->buffer));
}

int USBDeluxeDevice_MIDI_Write(USBDeluxeDevice_MIDIContext *midi_ctx, uint8_t *buf, uint8_t len) {
	if (USBHandleBusy(midi_ctx->HandleTx)) {
		return -1;
	}

	midi_ctx->HandleTx = USBTxOnePacket(midi_ctx->USB_EP_MS, buf, len);

	return 0;
}

void USBDeluxe_DeviceDescriptor_InsertMIDISpecificEndpoint_OUT() {
	/* MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor */
	const uint8_t buf0[] = {
		0x5,					// Size of this descriptor in bytes
		0x25,					// bDescriptorType - CS_ENDPOINT
		0x01,					// bDescriptorSubtype - MS_GENERAL
		0x01,					// bNumEmbMIDIJack
		0x01,					// BaAssocJackID(1)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertMIDISpecificEndpoint_IN() {
	/* MIDI Adapter Class-specific Bulk IN Endpoint Descriptor */
	const uint8_t buf0[] = {
		0x5,					// Size of this descriptor in bytes
		0x25,					// bDescriptorType - CS_ENDPOINT
		0x01,					// bDescriptorSubtype - MS_GENERAL
		0x01,					// bNumEmbMIDIJack
		0x03,					// BaAssocJackID(1)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertMIDISpecific_AC() {
	/* MIDI Adapter Class-specific AC Interface Descriptor */
	const uint8_t buf0[] = {
		0x9,					// Size of this descriptor in bytes
		0x24,					// bDescriptorType - CS_INTERFACE
		0x01,					// bDescriptorSubtype - HEADER
		0x00,					// bcdADC
		0x01,					// bcdADC
		0x09,					// wTotalLength
		0x00,					// wTotalLength
		0x01,					// bInCollection
		0x01,					// baInterfaceNr(1)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertMIDISpecific_MS() {
	/* MIDI Adapter Class-specific MS Interface Descriptor */
	const uint8_t buf0[] = {
		0x7,                                        // Size of this descriptor in bytes
		0x24,                                        // bDescriptorType - CS_INTERFACE
		0x01,                                        // bDescriptorSubtype - MS_HEADER
		0x00,                                        // bcdADC
		0x01,                                        // bcdADC
		0x41,                                        // wTotalLength
		0x00,                                        // wTotalLength
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertMIDISpecific_JACK() {
	const uint8_t buf0[] = {
		/* MIDI Adapter MIDI IN Jack Descriptor (Embedded) */
		0x6,                                        // Size of this descriptor in bytes
		0x24,                                        // bDescriptorType - CS_INTERFACE
		0x02,                                        // bDescriptorSubtype - MIDI_IN_JACK
		0x01,                                        // bJackType - EMBEDDED
		0x01,                                        // bJackID
		0x00,                                        // iJack

		/* MIDI Adapter MIDI IN Jack Descriptor (External) */
		0x6,                                       // Size of this descriptor in bytes
		0x24,                                        // bDescriptorType - CS_INTERFACE
		0x02,                                        // bDescriptorSubtype - MIDI_IN_JACK
		0x02,                                        // bJackType - EXTERNAL
		0x02,                                        // bJackID
		0x00,                                        // iJack

		/* MIDI Adapter MIDI OUT Jack Descriptor (Embedded) */
		0x9,                                       // Size of this descriptor in bytes
		0x24,                                        // bDescriptorType - CS_INTERFACE
		0x03,                                        // bDescriptorSubtype - MIDI_OUT_JACK
		0x01,                                        // bJackType - EMBEDDED
		0x03,                                        // bJackID
		0x01,                                        // bNrInputPins
		0x02,                                        // BaSourceID(1)
		0x01,                                        // BaSourcePin(1)
		0x00,                                        // iJack

		/* MIDI Adapter MIDI OUT Jack Descriptor (External) */
		0x9,                                       // Size of this descriptor in bytes
		0x24,                                        // bDescriptorType - CS_INTERFACE
		0x03,                                        // bDescriptorSubtype - MIDI_OUT_JACK
		0x02,                                        // bJackType - EXTERNAL
		0x04,                                        // bJackID
		0x01,                                        // bNrInputPins
		0x01,                                        // BaSourceID(1)
		0x01,                                        // BaSourcePin(1)
		0x00,                                        // iJack
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

uint8_t USBDeluxe_DeviceFunction_Add_MIDI(void *userp, USBDeluxeDevice_MIDI_Operations *midi_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t iface_ac = USBDeluxe_DeviceDescriptor_InsertInterface(0, 0, 0x1, 0x1, 0x0);
	USBDeluxe_DeviceDescriptor_InsertMIDISpecific_AC();
	uint8_t iface_ms = USBDeluxe_DeviceDescriptor_InsertInterface(0, 2, 0x1, 0x3, 0x0);
	USBDeluxe_DeviceDescriptor_InsertMIDISpecific_MS();

	USBDeluxe_DeviceDescriptor_InsertMIDISpecific_JACK();

	uint8_t next_free_ep = usb_device_desc_ctx.used_endpoints;
	USBDeluxe_DeviceDescriptor_InsertEndpointRaw(next_free_ep, _BULK, 0x40, 0, 0, 0);
	USBDeluxe_DeviceDescriptor_InsertMIDISpecificEndpoint_OUT();

	USBDeluxe_DeviceDescriptor_InsertEndpointRaw(next_free_ep | 0x80, _BULK, 0x40, 0, 0, 0);
	USBDeluxe_DeviceDescriptor_InsertMIDISpecificEndpoint_IN();
	usb_device_desc_ctx.used_endpoints++;


	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_MIDI);

	USBDeluxeDevice_MIDI_Create(ctx->drv_ctx, userp, iface_ac, iface_ms, next_free_ep, midi_ops);

	return last_idx;
}

#endif
