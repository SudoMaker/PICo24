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

#include "usb_deluxe_host.h"
#include "../usb_deluxe.h"

#include "usb_host_hid.h"

#ifdef PICo24_Enable_Peripheral_USB_HOST

void USBDeluxe_Host_Init() {
	usb_deluxe_role = USB_ROLE_HOST;

	USBHostInit(0);
}

void USBDeluxe_HostDriverTasks() {
	for (uint16_t i=0; i<usbClientDrvTable.size; i++) {
		switch (((CLIENT_DRIVER_TABLE *)usbClientDrvTable.data)[i].func) {
			case USB_FUNC_HID:
				USBDeluxeHost_HID_Tasks(((CLIENT_DRIVER_TABLE *)usbClientDrvTable.data)[i].dev_addr);
			case USB_FUNC_HUB:
				USBHostHubTasks();
			default:
				break;

		}
	}
}

uint16_t USBDeluxe_Host_AddDriver(uint8_t func, USB_CLIENT_INIT init_cb, USB_CLIENT_EVENT_HANDLER event_cb, USB_CLIENT_EVENT_HANDLER dataevent_cb, uint32_t flags) {
	uint16_t last_idx = usbClientDrvTable.size;

	CLIENT_DRIVER_TABLE *ctx = Vector_EmplaceBack(&usbClientDrvTable);
	ctx->func = func;
	ctx->Initialize = init_cb;
	ctx->EventHandler = event_cb;
	ctx->DataEventHandler = dataevent_cb;
	ctx->flags = flags;

	return last_idx;
}

uint16_t USBDeluxe_Host_AddDeviceClass(uint16_t driver_id, uint8_t bConfiguration, uint8_t bClass, uint8_t bSubClass, uint8_t bProtocol) {
	uint16_t last_idx = usbTPL.size;

	USB_TPL *ctx = Vector_EmplaceBack(&usbTPL);
	ctx->ClientDriver = driver_id;
	ctx->device.bClass = bClass;
	ctx->device.bSubClass = bSubClass;
	ctx->device.bProtocol = bProtocol;

	ctx->bConfiguration = bConfiguration;
	ctx->flags.bfIsClassDriver = 1;

	return last_idx;
}

#endif