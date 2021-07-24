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
*/

#pragma once

#include <stdlib.h>
#include <string.h>

#include "../usb.h"
#include "usb_device.h"

#include "usb_deluxe_device_cdc_acm.h"
#include "usb_deluxe_device_msd.h"
#include "usb_deluxe_device_hid.h"
#include "usb_deluxe_device_midi.h"
#include "usb_deluxe_device_audio.h"
#include "usb_deluxe_device_cdc_ecm.h"
#include "usb_deluxe_device_cdc_ncm.h"


typedef enum {
	USB_EP_DIR_IN = 0x1,
	USB_EP_DIR_OUT = 0x2
} USBEndpointDirection;

typedef struct {
	Vector raw;

	uint16_t used_interfaces;
	uint16_t used_endpoints;
} USBDeviceDescriptorContext;

#define USBDeviceStringDescriptor_FieldSize		16

typedef struct {
	uint8_t bLength;
	uint8_t bDscType;
	uint16_t string[USBDeviceStringDescriptor_FieldSize];
} USBDeviceStringDescriptor;

typedef struct {
	uint16_t func;
	void *drv_ctx;
} USBDeviceDriverContext;

extern USB_DEVICE_DESCRIPTOR usb_device_desc;

extern USBDeviceDescriptorContext usb_device_desc_ctx;
extern Vector usb_device_driver_ctx;

extern Vector usb_device_string_descriptor_table;

extern void USBDeluxe_Device_ConfigInit(uint16_t vid, uint16_t pid, const char *manufacturer_str, const char *product_str, const char *serial_str);
extern void USBDeluxe_Device_ConfigApply();

extern void USBDeluxe_DeviceDescriptor_Init();
extern void USBDeluxe_DeviceDescriptor_RefreshSize();

extern uint16_t USBDeluxe_DeviceDescriptor_AddString(const char *str, int len);
extern uint16_t USBDeluxe_DeviceDescriptor_AddString16(const uint16_t *str, size_t len);

extern void USBDeluxe_Device_EventInit();
extern void USBDeluxe_Device_EventCheckRequest();

extern USBDeviceDriverContext *USBDeluxe_DeviceGetDriverContext(uint8_t index);
extern USBDeviceDriverContext *USBDeluxe_DeviceDriver_AllocateMemory(uint8_t usb_func);

extern void USBDeluxe_Device_Tasks();

extern void USBDeluxe_Device_TaskCreate(uint16_t idx, const char *tag);

extern uint8_t USBDeluxe_DeviceDescriptor_InsertInterface(uint8_t alternate_setting, uint8_t nr_endpoints, uint8_t class, uint8_t subclass, uint8_t protocol);
extern void USBDeluxe_DeviceDescriptor_InsertEndpointRaw(uint8_t addr, uint8_t attr, uint16_t size, uint8_t interval,
							 int16_t opt_refresh, int16_t opt_sync_addr);
extern uint8_t USBDeluxe_DeviceDescriptor_InsertEndpoint(uint8_t direction, uint8_t attr, uint16_t size, uint8_t interval,
							 int16_t opt_refresh, int16_t opt_sync_addr);
extern void USBDeluxe_DeviceDescriptor_InsertIAD(uint8_t iface, uint8_t nr_contiguous_ifaces, uint8_t iface_class, uint16_t iface_subclass, uint8_t protocol);

