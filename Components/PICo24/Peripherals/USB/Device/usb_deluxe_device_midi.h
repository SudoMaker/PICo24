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

typedef struct {
	void (*RxDone)(void *userp, uint8_t *buf, uint16_t len);
} USBDeluxeDevice_MIDI_Operations;

typedef struct {
	void *userp;

	uint8_t buffer[64];
	uint8_t USB_IFACE_AC, USB_IFACE_MS;
	uint8_t USB_EP_MS;

	USB_HANDLE *HandleRx, *HandleTx;

	USBDeluxeDevice_MIDI_Operations ops;
} USBDeluxeDevice_MIDIContext;

extern void USBDeluxeDevice_MIDI_Create(USBDeluxeDevice_MIDIContext *midi_ctx, void *userp, uint8_t usb_iface_ac, uint8_t usb_iface_ms, uint8_t usb_ep_ms, USBDeluxeDevice_MIDI_Operations *midi_ops);
extern void USBDeluxeDevice_MIDI_Init(USBDeluxeDevice_MIDIContext *midi_ctx);
extern void USBDeluxeDevice_MIDI_Tasks(USBDeluxeDevice_MIDIContext *midi_ctx);
extern int USBDeluxeDevice_MIDI_Write(USBDeluxeDevice_MIDIContext *midi_ctx, uint8_t *buf, uint8_t len);

extern uint8_t USBDeluxe_DeviceFunction_Add_MIDI(void *userp, USBDeluxeDevice_MIDI_Operations *midi_ops);
