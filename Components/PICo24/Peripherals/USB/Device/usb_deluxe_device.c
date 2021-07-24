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

#include <stdio.h>

#include "usb_deluxe_device.h"
#include "../usb_deluxe.h"

#include <PICo24/Library/Vector.h>
#include <PICo24/Library/SafeMalloc.h>

#include <PICo24/Core/FreeRTOS_Support.h>

#ifdef PICo24_Enable_Peripheral_USB_DEVICE

USBDeviceDescriptorContext usb_device_desc_ctx;

Vector usb_device_driver_ctx = {
	.size = 0,
	.element_size = sizeof(USBDeviceDriverContext),
	.data = NULL
};

USB_DEVICE_DESCRIPTOR usb_device_desc = {
	0x12,			// Size of this descriptor in bytes
	USB_DESCRIPTOR_DEVICE,		// DEVICE descriptor type
	0x0200,			// USB Spec Release Number in BCD format
	0xEF,		// Class Code "MISC_DEVICE" (ex: uses IAD descriptor)
	0x02,		// Subclass code
	0x01,		// Protocol code
	USB_EP0_BUFF_SIZE,		// Max packet size for EP0, see usb_config.h
	0x04D8,			// Vendor ID
	0x0057,		// Product ID
	0x0001,		// Device release number in BCD format
	0x01,		// Manufacturer string index
	0x02,			// Product string index
	0x03,		// Device serial number string index
	0x01		// Number of possible configurations
};

Vector usb_device_string_descriptor_table = {
	.data = NULL,
	.size = 0,
	.element_size = sizeof(USBDeviceStringDescriptor *)
};

void USBDeluxe_Device_ConfigInit(uint16_t vid, uint16_t pid, const char *manufacturer_str, const char *product_str, const char *serial_str) {
	USBDeluxe_DeviceDescriptor_Init();

	usb_device_desc.idVendor = vid;
	usb_device_desc.idProduct = pid;

	uint16_t lang_code = 0x0409;
	USBDeluxe_DeviceDescriptor_AddString16(&lang_code, 1);

	USBDeluxe_DeviceDescriptor_AddString(manufacturer_str, -1);
	USBDeluxe_DeviceDescriptor_AddString(product_str, -1);
	USBDeluxe_DeviceDescriptor_AddString(serial_str, -1);
}

void USBDeluxe_Device_ConfigApply() {
	USBDeluxe_DeviceDescriptor_RefreshSize();
}

void USBDeluxe_DeviceDescriptor_Init() {
	const uint8_t desc_hdr[] = {
		0x09,					// Size of this descriptor in bytes
		USB_DESCRIPTOR_CONFIGURATION,		// CONFIGURATION descriptor type
		0x09, 0x00,				// Total length of data for this cfg, **little endian**
		1,					// Number of interfaces in this cfg
		1,					// Index value of this configuration
		0,					// Configuration string index
		_DEFAULT|_SELF,				// Attributes, see usbdefs_std_dsc.h
		50,					// Max power consumption (2X mA)
	};

	memset(&usb_device_desc_ctx, 0, sizeof(USBDeviceDescriptorContext));
	usb_device_desc_ctx.raw.element_size = 1;
	usb_device_desc_ctx.used_endpoints = 1;

	Vector_PushBack2(&usb_device_desc_ctx.raw, desc_hdr, sizeof(desc_hdr));
}

extern uint8_t *USB_CD_Ptr[1];

void USBDeluxe_DeviceDescriptor_RefreshSize() {
	uint16_t len = usb_device_desc_ctx.raw.size;
	((uint8_t *)usb_device_desc_ctx.raw.data)[2] = len & 0xff;
	((uint8_t *)usb_device_desc_ctx.raw.data)[3] = len >> 8;
	((uint8_t *)usb_device_desc_ctx.raw.data)[4] = usb_device_desc_ctx.used_interfaces;

	USB_CD_Ptr[0] = usb_device_desc_ctx.raw.data;
}

uint16_t USBDeluxe_DeviceDescriptor_AddString(const char *str, int len) {
	if (len == -1) {
		len = strlen(str);
	}

	size_t desc_len = 2 + 2 * len;
	USBDeviceStringDescriptor *desc = malloc(2 + 2 * len);

	desc->bDscType = USB_DESCRIPTOR_STRING;
	desc->bLength = desc_len;

	for (size_t i=0; i<len; i++) {
		desc->string[i] = ((uint8_t *)str)[i];
	}

//	printf("USBDeluxe_DeviceDescriptor_AddString: idx %u, str %s, len %u\n", usb_device_string_descriptor_table.size, str, desc->bLength);

	Vector_PushBack(&usb_device_string_descriptor_table, &desc);

	return usb_device_string_descriptor_table.size - 1;
}

uint16_t USBDeluxe_DeviceDescriptor_AddString16(const uint16_t *str, size_t len) {
	size_t desc_len = 2 + 2 * len;
	USBDeviceStringDescriptor *desc = malloc(desc_len);

	desc->bDscType = USB_DESCRIPTOR_STRING;
	desc->bLength = desc_len;

	for (size_t i=0; i<len; i++) {
		desc->string[i] = str[i];
	}

//	printf("USBDeluxe_DeviceDescriptor_AddString16: idx %u, len %u\n", usb_device_string_descriptor_table.size, desc->bLength);

	Vector_PushBack(&usb_device_string_descriptor_table, &desc);

	return usb_device_string_descriptor_table.size - 1;
}

uint8_t USBDeluxe_DeviceDescriptor_InsertInterface(uint8_t alternate_setting, uint8_t nr_endpoints, uint8_t class, uint8_t subclass, uint8_t protocol) {
	uint8_t ret = usb_device_desc_ctx.used_interfaces;

	uint8_t buf[] = {
		0x9,					// Size of this descriptor in bytes
		USB_DESCRIPTOR_INTERFACE,		// INTERFACE descriptor type
		usb_device_desc_ctx.used_interfaces,	// Interface Number
		alternate_setting,			// Alternate Setting Number
		nr_endpoints,				// Number of endpoints in this intf
		class,					// Class code
		subclass,				// Subclass code
		protocol,				// Protocol code
		0x0					// Interface string index
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf, sizeof(buf));
	usb_device_desc_ctx.used_interfaces += 1;

	return ret;
}

void USBDeluxe_DeviceDescriptor_InsertEndpointRaw(uint8_t addr, uint8_t attr, uint16_t size, uint8_t interval,
						  int16_t opt_refresh, int16_t opt_sync_addr) {
	uint8_t len = 0x7;

	if (opt_refresh != -1) {
		len++;
	}
	if (opt_sync_addr != -1) {
		len++;
	}

	uint8_t buf[] = {
		len,					// Size of this descriptor in bytes
		USB_DESCRIPTOR_ENDPOINT,		// Endpoint Descriptor
		addr,					// EndpointAddress
		attr,					// Attributes
		size & 0xff,				// Size, <7:0>
		size >> 8,				// Size, <15:8>
		interval,				// Interval
		opt_refresh,
		opt_sync_addr
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf, buf[0]);
}

uint8_t USBDeluxe_DeviceDescriptor_InsertEndpoint(uint8_t direction, uint8_t attr, uint16_t size, uint8_t interval, int16_t opt_refresh, int16_t opt_sync_addr) {
	uint8_t next_free_ep = usb_device_desc_ctx.used_endpoints;

	if (direction & USB_EP_DIR_IN) {
		USBDeluxe_DeviceDescriptor_InsertEndpointRaw(next_free_ep | 0x80, attr, size, interval, opt_refresh, opt_sync_addr);
	}

	if (direction & USB_EP_DIR_OUT) {
		USBDeluxe_DeviceDescriptor_InsertEndpointRaw(next_free_ep, attr, size, interval, opt_refresh, opt_sync_addr);
	}

	usb_device_desc_ctx.used_endpoints++;
	return next_free_ep;
}

void USBDeluxe_DeviceDescriptor_InsertIAD(uint8_t iface, uint8_t nr_contiguous_ifaces, uint8_t iface_class, uint16_t iface_subclass, uint8_t protocol) {
	uint8_t buf0[] = {
		0x8,					// Size of this descriptor in bytes
		0x0b,					// Interface assocication descriptor type
		iface,					// The first associated interface
		nr_contiguous_ifaces,			// Number of contiguous associated interface
		iface_class,				// bInterfaceClass of the first interface
		iface_subclass,				// bInterfaceSubclass of the first interface
		protocol,				// bInterfaceProtocol of the first interface
		0,					// Interface string index
	};

	Vector_PushBack2(&usb_device_desc_ctx.raw, buf0, sizeof(buf0));
}

USBDeviceDriverContext *USBDeluxe_DeviceDriver_AllocateMemory(uint8_t usb_func) {
	USBDeviceDriverContext *ctx = Vector_EmplaceBack(&usb_device_driver_ctx);

	switch (usb_func) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID
		case USB_FUNC_HID:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_HIDContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO
		case USB_FUNC_AUDIO:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_AudioContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MIDI
		case USB_FUNC_MIDI:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_MIDIContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
		case USB_FUNC_CDC_ACM:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_CDCACMContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM
		case USB_FUNC_CDC_ECM:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_CDCECMContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
		case USB_FUNC_CDC_NCM:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_CDCNCMContext));
			break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD
		case USB_FUNC_MSD:
			ctx->drv_ctx = calloc_safe(1, sizeof(USBDeluxeDevice_MSDContext));
			break;
#endif
		default:
			break;
	}

	ctx->func = usb_func;

	return ctx;
}

inline __attribute__((always_inline)) USBDeviceDriverContext *USBDeluxe_DeviceGetDriverContext(uint8_t index) {
	return Vector_At(&usb_device_driver_ctx, index);
}

void USBDeluxe_Device_EventInit() {
	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		USBDeviceDriverContext *drv_ctx = USBDeluxe_DeviceGetDriverContext(i);

		switch (drv_ctx->func) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO
			case USB_FUNC_AUDIO:
				USBDeluxeDevice_Audio_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MIDI
			case USB_FUNC_MIDI:
				USBDeluxeDevice_MIDI_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
			case USB_FUNC_CDC_ACM:
				USBDeluxeDevice_CDC_ACM_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM
			case USB_FUNC_CDC_ECM:
				USBDeluxeDevice_CDC_ECM_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
			case USB_FUNC_CDC_NCM:
				USBDeluxeDevice_CDC_NCM_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID
			case USB_FUNC_HID:
				USBDeluxeDevice_HID_Init(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD
			case USB_FUNC_MSD:
				USBDeluxeDevice_MSD_Init(drv_ctx->drv_ctx);
				break;
#endif
			default:
				break;
		}
	}
}

void USBDeluxe_Device_EventCheckRequest() {
	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		USBDeviceDriverContext *drv_ctx = USBDeluxe_DeviceGetDriverContext(i);

		switch (drv_ctx->func) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
			case USB_FUNC_CDC_ACM:
				USBDeluxeDevice_CDC_ACM_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM
			case USB_FUNC_CDC_ECM:
				USBDeluxeDevice_CDC_ECM_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
			case USB_FUNC_CDC_NCM:
				USBDeluxeDevice_CDC_NCM_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO
			case USB_FUNC_AUDIO:
				USBDeluxeDevice_Audio_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID
			case USB_FUNC_HID:
				USBDeluxeDevice_HID_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD
			case USB_FUNC_MSD:
				USBDeluxeDevice_MSD_CheckRequest(drv_ctx->drv_ctx);
				break;
#endif
			default:
				break;
		}
	}
}

void USBDeluxe_Device_Tasks() {
	if (USBGetDeviceState() < CONFIGURED_STATE) {
		return;
	}

	if (USBIsDeviceSuspended()== true) {
		return;
	}

	for (size_t i=0; i<usb_device_driver_ctx.size; i++) {
		USBDeviceDriverContext *drv_ctx = USBDeluxe_DeviceGetDriverContext(i);

		switch (drv_ctx->func) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID
			case USB_FUNC_HID:
				USBDeluxeDevice_HID_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO
			case USB_FUNC_AUDIO:
				USBDeluxeDevice_Audio_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MIDI
			case USB_FUNC_MIDI:
				USBDeluxeDevice_MIDI_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
			case USB_FUNC_CDC_ACM:
				USBDeluxeDevice_CDC_ACM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM
			case USB_FUNC_CDC_ECM:
				USBDeluxeDevice_CDC_ECM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
			case USB_FUNC_CDC_NCM:
				USBDeluxeDevice_CDC_NCM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD
			case USB_FUNC_MSD:
				USBDeluxeDevice_MSD_Tasks(drv_ctx->drv_ctx);
				break;
#endif
			default:
				break;
		}
	}
}

void USBDeluxe_Device_Task(void *p) {
	taskENTER_CRITICAL();
	uint16_t idx = (uint16_t)p;
	USBDeviceDriverContext *drv_ctx = USBDeluxe_DeviceGetDriverContext(idx);
	printf("USBDeluxe_Device_Task: Started, driver index: %u, ctx: 0x%04x\n", idx, drv_ctx);
	taskEXIT_CRITICAL();

	while (1) {
//		printf("ut\n");
//		vTaskSuspendAll();

//		taskENTER_CRITICAL();

		switch (drv_ctx->func) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_HID
			case USB_FUNC_HID:
				USBDeluxeDevice_HID_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_AUDIO
			case USB_FUNC_AUDIO:
				USBDeluxeDevice_Audio_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MIDI
			case USB_FUNC_MIDI:
				USBDeluxeDevice_MIDI_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ACM
			case USB_FUNC_CDC_ACM:
				USBDeluxeDevice_CDC_ACM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_ECM
			case USB_FUNC_CDC_ECM:
				USBDeluxeDevice_CDC_ECM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_CDC_NCM
			case USB_FUNC_CDC_NCM:
				USBDeluxeDevice_CDC_NCM_Tasks(drv_ctx->drv_ctx);
				break;
#endif
#ifdef PICo24_Enable_Peripheral_USB_DEVICE_MSD
			case USB_FUNC_MSD:
				USBDeluxeDevice_MSD_Tasks(drv_ctx->drv_ctx);
				break;
#endif
			default:
				break;
		}

//		if (xTaskResumeAll() == pdTRUE) {
//			/* A context switch occurred within xTaskResumeAll(). */
//		} else {
//			/* A context switch did not occur within xTaskResumeAll(). */
//			taskYIELD();
//		}

		taskYIELD();

//		taskEXIT_CRITICAL();
	}
}

void USBDeluxe_Device_TaskCreate(uint16_t idx, const char *tag) {
	char name[16];
	sprintf(name, "USB %u: %s", idx, tag);

	xTaskCreate(USBDeluxe_Device_Task, name, 384, (void *)idx, 3, NULL);
}

#endif
