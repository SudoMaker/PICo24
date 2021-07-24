/*
    This file is part of PICo24 SDK.

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
*/

#pragma once

#include "usb_host.h"
#include "usb_deluxe_host_hid.h"
#include "usb_deluxe_host_hub.h"
#include "usb_host_cdc.h"

#include <PICo24/Library/DebugTools.h>

extern void USBDeluxe_Host_Init();
extern void USBDeluxe_HostDriverTasks();

extern uint16_t USBDeluxe_Host_AddDriver(uint8_t func, USB_CLIENT_INIT init_cb, USB_CLIENT_EVENT_HANDLER event_cb, USB_CLIENT_EVENT_HANDLER dataevent_cb, uint32_t flags);
extern uint16_t USBDeluxe_Host_AddDeviceClass(uint16_t driver_id, uint8_t bConfiguration, uint8_t bClass, uint8_t bSubClass, uint8_t bProtocol);