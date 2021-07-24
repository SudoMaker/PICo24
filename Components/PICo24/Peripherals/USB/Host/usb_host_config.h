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

// Created by the Microchip USBConfig Utility, Version 1.0.4.0, 4/25/2008, 17:05:22

#ifndef _usb_host_config_h_
#define _usb_host_config_h_

// Supported USB Configurations


#define USB_SUPPORT_HOST

// Hardware Configuration

#define USB_PING_PONG_MODE  USB_PING_PONG__FULL_PING_PONG

// Host Configuration

#define NUM_TPL_ENTRIES usbTPLSize
#define USB_NUM_CONTROL_NAKS 20
#define USB_NUM_INTERRUPT_NAKS 20
#define USB_INITIAL_VBUS_CURRENT (100/2)
#define USB_INSERT_TIME (250+1)
#define USB_HOST_APP_EVENT_HANDLER USB_Host_EventHandler
#define USB_ENABLE_TRANSFER_EVENT

#define USB_SUPPORT_INTERRUPT_TRANSFERS
#define USB_SUPPORT_ISOCHRONOUS_TRANSFERS
#define USB_SUPPORT_BULK_TRANSFERS

// Helpful Macros

#define USBInitialize()            \
    {                               \
        USBHostInit(0);             \
    }


#endif

