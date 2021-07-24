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
#include "usb_deluxe_device_audio.h"

extern volatile CTRL_TRF_SETUP SetupPkt;

#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO

void USBDeluxeDevice_Audio_Create(USBDeluxeDevice_AudioContext *audio_ctx, void *userp, uint8_t usb_iface_ac, uint8_t usb_iface_as, uint8_t usb_ep_as, USBDeluxeDevice_Audio_Operations *audio_ops) {
	memset(audio_ctx, 0, sizeof(USBDeluxeDevice_MIDIContext));

	audio_ctx->userp = userp;
	audio_ctx->USB_IFACE_AC = usb_iface_ac;
	audio_ctx->USB_IFACE_AS = usb_iface_as;
	audio_ctx->USB_EP_AS = usb_ep_as;

	memcpy(&audio_ctx->ops, audio_ops, sizeof(USBDeluxeDevice_Audio_Operations));
}

void USBDeluxeDevice_Audio_Init(USBDeluxeDevice_AudioContext *audio_ctx) {
	USBEnableEndpoint(audio_ctx->USB_EP_AS, USB_OUT_ENABLED | USB_IN_ENABLED | USB_DISALLOW_SETUP);
//	audio_ctx->HandleRx = USBRxOnePacket(audio_ctx->USB_EP_AS, audio_ctx->buffer, sizeof(audio_ctx->buffer));
}



void USBDeluxeDevice_Audio_CheckRequest(USBDeluxeDevice_AudioContext *audio_ctx) {
	/*
	 * If request recipient is not an interface then return
	 */
	if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;
	/*
	 * If request type is not class-specific then return
	 */
	if(SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD) return;
	/*
	 * Interface ID must match interface numbers associated with
	 * Audio class, else return
	 */
	if((SetupPkt.bIntfID != audio_ctx->USB_IFACE_AC)&&
	   (SetupPkt.bIntfID != audio_ctx->USB_IFACE_AS)) return;

	switch(SetupPkt.wIndex >> 8)// checking for the Entity ID (Entity ID are defined in the config.h file)
	{
		case ID_INPUT_TERMINAL:
			audio_ctx->ops.InputTerminalControlRequestsHandler(audio_ctx->userp,
									   SetupPkt.W_Value.Val,
									   SetupPkt.W_Index.byte.HB,
									   SetupPkt.W_Index.byte.LB,
									   SetupPkt.wLength);
			break;
		case ID_OUTPUT_TERMINAL:
			audio_ctx->ops.OutputTerminalControlRequestsHandler(audio_ctx->userp,
									    SetupPkt.W_Value.Val,
									    SetupPkt.W_Index.byte.HB,
									    SetupPkt.W_Index.byte.LB,
									    SetupPkt.wLength);
			break;
		case ID_MIXER_UNIT:
			audio_ctx->ops.MixerUnitControlRequestsHandler(audio_ctx->userp,
								       SetupPkt.W_Value.byte.HB,
								       SetupPkt.W_Value.byte.LB,
								       SetupPkt.W_Index.byte.HB,
								       SetupPkt.W_Index.byte.LB,
								       SetupPkt.wLength);
			break;
		case ID_SELECTOR_UNIT:
			audio_ctx->ops.SelectorUnitControlRequestsHabdler(audio_ctx->userp,
									  SetupPkt.W_Index.byte.HB,
									  SetupPkt.W_Index.byte.LB,
									  SetupPkt.wLength);
			break;
		case ID_FEATURE_UNIT:
			audio_ctx->ops.FeatureUnitControlRequestsHandler(audio_ctx->userp,
									 SetupPkt.W_Value.byte.HB,
									 SetupPkt.W_Value.byte.LB,
									 SetupPkt.W_Index.byte.HB,
									 SetupPkt.W_Index.byte.LB,
									 SetupPkt.wLength);
			break;
		case ID_PROCESSING_UNIT:
			audio_ctx->ops.ProcessingUnitControlRequestsHandler(audio_ctx->userp,
									    SetupPkt.W_Value.Val,
									    SetupPkt.W_Index.byte.HB,
									    SetupPkt.W_Index.byte.LB,
									    SetupPkt.wLength);
			break;
		case ID_EXTENSION_UNIT:
			audio_ctx->ops.ExtensionUnitControlRequestsHandler(audio_ctx->userp,
									   SetupPkt.W_Value.Val,
									   SetupPkt.W_Index.byte.HB,
									   SetupPkt.W_Index.byte.LB,
									   SetupPkt.wLength);
			break;
		default:
			break;

	}//end switch(SetupPkt.bRequest
}//end USBCheckAudioRequest


void USBDeluxeDevice_Audio_Tasks(USBDeluxeDevice_AudioContext *audio_ctx) {
	if (USBHandleBusy(audio_ctx->HandleRx)) {
		return;
	} else {
		uint8_t len = USBHandleGetLength(audio_ctx->HandleRx);
		if (audio_ctx->ops.RxDone) {
			audio_ctx->ops.RxDone(audio_ctx->userp, audio_ctx->buffer, len);
		}
	}

	audio_ctx->HandleRx = USBRxOnePacket(audio_ctx->USB_EP_AS, audio_ctx->buffer, sizeof(audio_ctx->buffer));
}

int USBDeluxeDevice_Audio_Write(USBDeluxeDevice_AudioContext *audio_ctx, uint8_t *buf, uint8_t len) {
	if (USBHandleBusy(audio_ctx->HandleTx)) {
		return -1;
	}

	audio_ctx->HandleTx = USBTxOnePacket(audio_ctx->USB_EP_AS, buf, len);

	return 0;
}

void USBDeluxe_DeviceDescriptor_InsertAudioSpecific_AC(uint8_t interface) {

	/* MIDI Adapter Class-specific AC Interface Descriptor */
	uint8_t buf0[] = {
		0x9,				// Size of this descriptor in bytes
		0x24,				// bDescriptorType - CS_INTERFACE
		0x01,				// bDescriptorSubtype - HEADER
		0x00,				// Audio Device compliant to the USB Audio specification version 1.00
		0x01,				// Audio Device compliant to the USB Audio specification version 1.00
		0x1e,				// wTotalLength
		0x00,				// wTotalLength
		0x01,				// The number of AudioStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs
		interface,			// AudioStreaming interface X belongs to this AudioControl interface
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertAudioSpecific_Terminal(uint16_t terminal_type, uint8_t nr_channels) {
	/* USB Microphone Input Terminal Descriptor */
	uint8_t buf0[] = {
		12,				// Size of this descriptor in bytes
		CS_INTERFACE,			// bDescriptorType - CS_INTERFACE
		INPUT_TERMINAL,			// OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
		ID_INPUT_TERMINAL,		// ID of this Terminal. (bTerminalID)
		terminal_type & 0xff,		// terminal_type
		terminal_type >> 8,		// terminal_type
		0x00,				// No association
		nr_channels,			// From Input Terminal.(bSourceID)
		0x00,				// Mono sets no position bits TODO
		0x00,				// Mono sets no position bits
		0x00,				// Unused
		0x00,				// Unused
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

	/* USB Microphone Output Terminal Descriptor */
	const uint8_t buf1[] = {
		0x9,				// Size of this descriptor in bytes
		CS_INTERFACE,			// bDescriptorType - CS_INTERFACE
		OUTPUT_TERMINAL,			// OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
		ID_OUTPUT_TERMINAL,		// ID of this Terminal. (bTerminalID)
		0x01,				// USB Streaming. (wTerminalType
		0x01,				// USB Streaming. (wTerminalType
		0x00,				// unused (bAssocTerminal)
		ID_INPUT_TERMINAL,		// From Input Terminal.(bSourceID)
		0x00,				// unused  (iTerminal)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf1, sizeof(buf1));

}

void USBDeluxe_DeviceDescriptor_InsertAudioSpecific_AS(uint8_t output_terminal) {
	/* USB Microphone Class-specific AS General Interface Descriptor */
	uint8_t buf0[] = {
		0x7,				// Size of this descriptor in bytes
		CS_INTERFACE,			// bDescriptorType - CS_INTERFACE
		AS_GENERAL,			// bDescriptorSubtype - HEADER
		output_terminal,			// Unit ID of the Output Terminal.(bTerminalLink)
		0x01,				// Interface delay. (bDelay)
		0x01,				// PCM Format (wFormatTag)
		0x00,				// PCM Format (wFormatTag)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertAudioSpecific_Type1Format(uint8_t nr_channels, uint8_t subframe_size, uint8_t bit_resolution, uint16_t sample_freq) {
	/* USB Microphone Type I Format Type Descriptor */

	uint8_t buf0[] = {
		11,				// Size of this descriptor in bytes
		CS_INTERFACE,			// bDescriptorType - CS_INTERFACE
		FORMAT_TYPE,			// FORMAT_TYPE subtype. (bDescriptorSubtype)
		0x01,				// FORMAT_TYPE_I. (bFormatType)
		nr_channels,			// channel.(bNrChannels)
		subframe_size,			// bytes per audio subframe.(bSubFrameSize)
		bit_resolution,			// bits per sample.(bBitResolution)
		0x01,				// One frequency supported. (bSamFreqType)
		sample_freq & 0xff,
		sample_freq >> 8,
		0x0,
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

void USBDeluxe_DeviceDescriptor_InsertAudioSpecific_IsocEP() {
	/* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor */

	const uint8_t buf0[] = {
		0x7,				// Size of this descriptor in bytes
		CS_INTERFACE,			// bDescriptorType - CS_INTERFACE
		AS_GENERAL,			// GENERAL subtype. (bDescriptorSubtype)
		0x00,				// No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
		0x00,				// Unused. (bLockDelayUnits)
		0x00,				// Unused. (wLockDelay)
		0x00,				// Unused. (wLockDelay)
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));

}

uint8_t USBDeluxe_DeviceFunction_Add_Audio(void *userp, USBDeluxeDevice_Audio_Operations *audio_ops) {
	uint8_t last_idx = usb_device_driver_ctx.size;

	uint8_t iface_ac = USBDeluxe_DeviceDescriptor_InsertInterface(0, 0, AUDIO_DEVICE, AUDIOCONTROL, 0x0);
	USBDeluxe_DeviceDescriptor_InsertAudioSpecific_AC(iface_ac + 1);
	USBDeluxe_DeviceDescriptor_InsertAudioSpecific_Terminal(0x0201, 1);

	USBDeluxe_DeviceDescriptor_InsertInterface(0, 0, AUDIO_DEVICE, AUDIOSTREAMING, 0x0);
	usb_device_desc_ctx.used_interfaces--;
	uint8_t iface_as = USBDeluxe_DeviceDescriptor_InsertInterface(1, 1, AUDIO_DEVICE, AUDIOSTREAMING, 0x0);

	USBDeluxe_DeviceDescriptor_InsertAudioSpecific_AS(ID_OUTPUT_TERMINAL);
	USBDeluxe_DeviceDescriptor_InsertAudioSpecific_Type1Format(1, 2, 16, 8000);

	uint8_t ep = USBDeluxe_DeviceDescriptor_InsertEndpoint(USB_EP_DIR_IN, _ISO, 16, 1, 0, 0);
	USBDeluxe_DeviceDescriptor_InsertAudioSpecific_IsocEP();


	USBDeviceDriverContext *ctx = USBDeluxe_DeviceDriver_AllocateMemory(USB_FUNC_AUDIO);

	USBDeluxeDevice_Audio_Create(ctx->drv_ctx, userp, iface_ac, iface_as, ep, audio_ops);

	return last_idx;
}

#endif
