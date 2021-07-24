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

#include "usb_deluxe_host.h"

extern bool USBHostHubInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID);
extern bool USBHostHubEventHandler(uint8_t address, USB_EVENT event, void *data, uint32_t size);
extern void USBHostHubTasks();

extern void USBDeluxe_HostDriver_Enable_Hub();