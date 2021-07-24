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

#include "usb_deluxe.h"

#include "Device/usb_deluxe_device.h"
#include "Host/usb_host.h"

#include <stdio.h>
#include <PICo24/Peripherals/USB/Host/usb_host_local.h>

USBDeluxe_Role usb_deluxe_role = USB_ROLE_NONE;

void USBDeluxe_Tasks() {
	if (usb_deluxe_role == USB_ROLE_DEVICE) {
		USBDeluxe_Device_Tasks();
	}
#ifdef PICo24_Enable_Peripheral_USB_HOST
	if (usb_deluxe_role == USB_ROLE_HOST) {
		USBDeluxe_HostDriverTasks();
		USBHostTasks();
	}
#endif
}

void USBDeluxe_SetRole(uint8_t role) {
	if (usb_deluxe_role != role) {
		USBDisableInterrupts();
		printf("Switching USB role to %s\n", role == USB_ROLE_HOST ? "host" : "device");

#ifdef PICo24_Enable_Peripheral_USB_HOST
		if (role == USB_ROLE_HOST) {
			USBHostInit(0);
			USBHostConfigureDeviceSlot(0, 1, 0);
			USBHostInit(1);
			USBHostConfigureDeviceSlot(1, 1, 0);
			USBHostTasks();
		}
#endif

		if (role == USB_ROLE_DEVICE) {
			USBDeviceInit();
			USBDeviceAttach();
		}

		printf("USB role switched to %s\n", role == USB_ROLE_HOST ? "host" : "device");
		usb_deluxe_role = role;
		USBEnableInterrupts();
	}
}
