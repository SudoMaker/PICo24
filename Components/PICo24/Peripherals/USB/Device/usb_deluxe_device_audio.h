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

/******Audio Interface Class Code**********/
#define AUDIO_DEVICE 0x01

/******* Audio Interface Subclass Codes**********/
#define AUDIOCONTROL 0x01
#define AUDIOSTREAMING 0x02
#define MIDISTREAMING 0x03

/******* Audio Class-Specific Descriptor Types**********/
#define CS_UNDEFINED 0x20
#define CS_DEVICE 0x21
#define CS_CONFIGURATION 0x22
#define CS_STRING 0x23
#define CS_INTERFACE 0x24
#define CS_ENDPOINT 0x25

/******* Audio Class-Specific AC Interface Descriptor Subtypes**********/
#define AC_DESCRIPTOR_UNDEFINED 0x00
#define HEADER 0x01
#define INPUT_TERMINAL 0x02
#define OUTPUT_TERMINAL 0x03
#define MIXER_UNIT 0x04
#define SELECTOR_UNIT 0x05
#define FEATURE_UNIT 0x06
#define PROCESSING_UNIT 0x07
#define EXTENSION_UNIT 0x08

/******* Audio Class-Specific AS Interface Descriptor Subtypes**********/
#define AS_DESCRIPTOR_UNDEFINED 0x00
#define AS_GENERAL 0x01
#define FORMAT_TYPE 0x02
#define FORMAT_SPECIFIC 0x03

/*******  Processing Unit Process Types **********/
#define PROCESS_UNDEFINED 0x00
#define UP_DOWNMIX_PROCESS 0x01
#define DOLBY_PROLOGIC_PROCESS 0x02
#define THREE_D_STEREO_EXTENDER_PROCESS 0x03
#define REVERBERATION_PROCESS 0x04
#define CHORUS_PROCESS 0x05
#define DYN_RANGE_COMP_PROCESS 0x06

/******* Audio Class-Specific Endpoint Descriptor Subtypes**********/
#define DESCRIPTOR_UNDEFINED 0x00
#define EP_GENERAL 0x01

/****** Audio Class-Specific Request Codes ************/
#define REQUEST_CODE_UNDEFINED 0x00
#define SET_CUR  0x01
#define SET_MIN  0x02
#define SET_MAX  0x03
#define SET_RES  0x04
#define SET_MEM  0x05
#define GET_CUR  0x81
#define GET_MIN  0x82
#define GET_MAX  0x83
#define GET_RES  0x84
#define GET_MEM  0x85
#define GET_STAT  0xFF

/************ Terminal Control Selectors ******/
#define TE_CONTROL_UNDEFINED 0x00
#define COPY_PROTECT_CONTROL 0x01

/************ Feature Unit Control Selectors ****/
#define FU_CONTROL_UNDEFINED 0x00
#define MUTE_CONTROL 0x01
#define VOLUME_CONTROL 0x02
#define BASS_CONTROL 0x03
#define MID_CONTROL 0x04
#define TREBLE_CONTROL 0x05
#define GRAPHIC_EQUALIZER_CONTROL 0x06
#define AUTOMATIC_GAIN_CONTROL 0x07
#define DELAY_CONTROL 0x08
#define BASS_BOOST_CONTROL 0x09
#define LOUDNESS_CONTROL 0x0A


/************  Processing Unit Control Selectors ****/
/*  Up/Down-mix Processing Unit Control Selectors */

#define UD_CONTROL_UNDEFINED 0x00
#define UD_ENABLE_CONTROL 0x01
#define UD_MODE_SELECT_CONTROL 0x02

/* Dolby Prologic(TM) Processing Unit Control Selectors */
#define DP_CONTROL_UNDEFINED 0x00
#define DP_ENABLE_CONTROL 0x01
#define DP_MODE_SELECT_CONTROL 0x02

/* Stereo Extender Processing Unit Control Selectors */
#define THREE_D_CONTROL_UNDEFINED 0x00
#define THREE_D_ENABLE_CONTROL 0x01
#define SPACIOUSNESS_CONTROL 0x03

/* Reverberation Processing Unit Control Selectors */
#define RV_CONTROL_UNDEFINED 0x00
#define RV_ENABLE_CONTROL 0x01
#define REVERB_LEVEL_CONTROL 0x02
#define REVERB_TIME_CONTROL 0x03
#define REVERB_FEEDBACK_CONTROL 0x04

/* Chorus Processing Unit Control Selectors */
#define CH_CONTROL_UNDEFINED 0x00
#define CH_ENABLE_CONTROL 0x01
#define CHORUS_LEVEL_CONTROL 0x02
#define CHORUS_RATE_CONTROL 0x03
#define CHORUS_DEPTH_CONTROL 0x04

/* Dynamic Range Compressor Processing Unit Control Selectors */
#define DR_CONTROL_UNDEFINED 0x00
#define DR_ENABLE_CONTROL 0x01
#define COMPRESSION_RATE_CONTROL 0x02
#define MAXAMPL_CONTROL 0x03
#define THRESHOLD_CONTROL 0x04
#define ATTACK_TIME 0x05
#define RELEASE_TIME 0x06


/************ Extension Unit Control Selectors **********/
#define XU_CONTROL_UNDEFINED 0x00
#define XU_ENABLE_CONTROL 0x01

/************ Endpoint Control Selectors **********/
#define EP_CONTROL_UNDEFINED 0x00
#define SAMPLING_FREQ_CONTROL 0x01
#define PITCH_CONTROL 0x02

/*********** Terminal Types***********************/
/*A complete list of Terminal Type codes is provided in
the document USB Audio Terminal Types */
#define USB_STREAMING 0x01, 0x01
#define MICROPHONE 0x01,0x02
#define SPEAKER 0x01,0x03
#define HEADPHONES 0x02,0x03

/************** Entity ID ***********/
#define ID_INPUT_TERMINAL  0x01
#define ID_OUTPUT_TERMINAL 0x02
#define ID_MIXER_UNIT      0x03
#define ID_SELECTOR_UNIT   0x04
#define ID_FEATURE_UNIT    0x05
#define ID_PROCESSING_UNIT 0x06
#define ID_EXTENSION_UNIT  0x07

typedef struct {
	void (*InputTerminalControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t input_terminal, uint8_t interface, uint16_t data_length);
	void (*OutputTerminalControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t output_terminal, uint8_t interface, uint16_t data_length);
	void (*MixerUnitControlRequestsHandler)(void *userp, uint8_t input_channel, uint8_t output_channel, uint8_t mixer_unit, uint8_t interface, uint16_t data_length);
	void (*SelectorUnitControlRequestsHabdler)(void *userp, uint8_t selector_unit, uint8_t interface, uint16_t data_length);
	void (*FeatureUnitControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t channel, uint8_t feature_unit, uint8_t interface, uint16_t data_length);
	void (*ProcessingUnitControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t processing_unit, uint8_t interface, uint16_t data_length);
	void (*ExtensionUnitControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t extension_unit, uint8_t interface, uint16_t data_length);
	void (*InterfaceControlRequestsHandler)(void *userp);
	void (*EndpointControlRequestsHandler)(void *userp, uint16_t control_selector, uint8_t endpoint, uint16_t data_length);
	void (*MemoryRequestsHandler)(void *userp, uint8_t endpoint, uint8_t entity, uint8_t interface, uint16_t data_length);
	void (*StatusRequestsHandler)(void *userp, uint8_t offset, uint8_t endpoint, uint8_t entity, uint8_t interface, uint16_t data_length);

	void (*RxDone)(void *userp, uint8_t *buf, uint16_t len);
} USBDeluxeDevice_Audio_Operations;

typedef struct {
	void *userp;

	uint8_t buffer[64];
	uint8_t USB_IFACE_AC, USB_IFACE_AS;
	uint8_t USB_EP_AS;

	USB_HANDLE *HandleRx, *HandleTx;

	USBDeluxeDevice_Audio_Operations ops;
} USBDeluxeDevice_AudioContext;

extern void USBDeluxeDevice_Audio_Create(USBDeluxeDevice_AudioContext *audio_ctx, void *userp, uint8_t usb_iface_ac, uint8_t usb_iface_as, uint8_t usb_ep_as, USBDeluxeDevice_Audio_Operations *audio_ops);
extern void USBDeluxeDevice_Audio_Init(USBDeluxeDevice_AudioContext *audio_ctx);

extern void USBDeluxeDevice_Audio_CheckRequest(USBDeluxeDevice_AudioContext *audio_ctx);
extern void USBDeluxeDevice_Audio_Tasks(USBDeluxeDevice_AudioContext *audio_ctx);
extern int USBDeluxeDevice_Audio_Write(USBDeluxeDevice_AudioContext *audio_ctx, uint8_t *buf, uint8_t len);

extern uint8_t USBDeluxe_DeviceFunction_Add_Audio(void *userp, USBDeluxeDevice_Audio_Operations *audio_ops);