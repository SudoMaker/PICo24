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
#include "usb_host_config.h"
#include "usb_host_hid.h"

#include <PICo24/Library/Vector.h>
#include <PICo24/Library/Set.h>
#include <PICo24/Library/Variant.h>

#include <stdio.h>

typedef struct {
	void (*RxDone)(void *userp, uint8_t interface, uint8_t *buf, uint16_t len);
	void (*ReportDescription)(void *userp, uint8_t interface, HID_REPORTITEM *report_item);
} USBDeluxeHost_HID_Operations;

typedef struct {
	uint8_t report_data[64];
	USBDeluxeHost_HID_Operations ops;
	void *userp;
	uint8_t task_state;
	uint8_t current_interface_pos;
	uint8_t interface_id;
} USBDeluxeHost_HIDContext;

extern void USBDeluxe_HostDriver_Enable_HID(void *userp, USBDeluxeHost_HID_Operations *hid_ops);
extern void USBDeluxeHost_HID_ReportDescHandler();
extern void USBDeluxeHost_HID_Tasks(uint8_t dev_addr);