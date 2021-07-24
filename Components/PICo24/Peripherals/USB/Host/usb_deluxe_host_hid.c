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

#include "usb_deluxe_host_hid.h"
#include "usb_deluxe_host.h"

#ifdef PICo24_Enable_Peripheral_USB_HOST

Vector hidhost_report_interface_ids = {
	.size = 0,
	.element_size = sizeof(uint8_t),
	.data = NULL
};

USBDeluxeHost_HIDContext usbdeluxe_host_hid_ctx = {0};

#define USAGE_PAGE_LEDS                 (0x08)
#define USAGE_PAGE_KEY_CODES            (0x07)
#define USAGE_PAGE_BUTTONS              (0x09)
#define USAGE_PAGE_GEN_DESKTOP          (0x01)

#define USAGE_MIN_MODIFIER_KEY          (0xE0)
#define USAGE_MAX_MODIFIER_KEY          (0xE7)

#define USAGE_MIN_NORMAL_KEY            (0x00)
#define USAGE_MAX_NORMAL_KEY            (0xFF)

void USBDeluxe_HostDriver_Enable_HID(void *userp, USBDeluxeHost_HID_Operations *hid_ops) {
	uint16_t host_hid = USBDeluxe_Host_AddDriver(USB_FUNC_HID, USBHostHIDInitialize, USBHostHIDEventHandler, NULL, 0);

	USBDeluxe_Host_AddDeviceClass(host_hid, 0, 3, 1, 1);
	USBDeluxe_Host_AddDeviceClass(host_hid, 0, 3, 0, 1);
	USBDeluxe_Host_AddDeviceClass(host_hid, 0, 3, 1, 2);
	USBDeluxe_Host_AddDeviceClass(host_hid, 0, 3, 0, 2);
	USBDeluxe_Host_AddDeviceClass(host_hid, 0, 3, 0, 0);

	usbdeluxe_host_hid_ctx.userp = userp;
	memcpy(&usbdeluxe_host_hid_ctx.ops, hid_ops, sizeof(USBDeluxeHost_HID_Operations));
}

void USBDeluxeHost_HID_ReportDescHandler() {
	USB_HID_DEVICE_RPT_INFO* pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
	USB_HID_ITEM_LIST* pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

	uint8_t NumOfReportItem = pDeviceRptinfo->reportItems;

	for (uint16_t i=0; i<NumOfReportItem; i++) {
		HID_REPORTITEM *reportItem = &pitemListPtrs->reportItemList[i];

//		printf("ReportItem %u: ID: %u, Type: %u, DataMode: %lu, UsagePage: %u\n", i,
//		       reportItem->globals.reportID, (uint8_t) reportItem->reportType, reportItem->dataModes, reportItem->globals.usagePage);

		uint8_t iface = USBHostHID_ApiGetCurrentInterfaceNum();

		if (usbdeluxe_host_hid_ctx.ops.ReportDescription) {
			usbdeluxe_host_hid_ctx.ops.ReportDescription(usbdeluxe_host_hid_ctx.userp, iface, reportItem);
		}

		Set_Insert(&hidhost_report_interface_ids, &iface);

	}
}

void USBDeluxeHost_HID_Tasks(uint8_t dev_addr) {
	uint8_t dev_status = USBHostHIDDeviceStatus(dev_addr);

	if (dev_status != USB_HID_NORMAL_RUNNING) {
		return;
	}

	usbdeluxe_host_hid_ctx.current_interface_pos %= hidhost_report_interface_ids.size;
	uint8_t interface_id = Variant_AsUint8(Vector_At(&hidhost_report_interface_ids, usbdeluxe_host_hid_ctx.current_interface_pos));

	if (usbdeluxe_host_hid_ctx.task_state == 0) {
		uint8_t rc = USBHostHIDRead(dev_addr, 0, interface_id, 64, usbdeluxe_host_hid_ctx.report_data);
		if (rc) {
			printf("HID read err: %u\n", rc);
		} else {
			usbdeluxe_host_hid_ctx.task_state = 1;
		}
	} else if (usbdeluxe_host_hid_ctx.task_state == 1) {
		uint8_t err_code, rc_read;
		if (USBHostHIDReadIsComplete(dev_addr, &err_code, &rc_read)) {
			if (rc_read) {
				if (rc_read > sizeof(usbdeluxe_host_hid_ctx.report_data)) {
					rc_read = sizeof(usbdeluxe_host_hid_ctx.report_data);
				}

				if (usbdeluxe_host_hid_ctx.ops.RxDone) {
					usbdeluxe_host_hid_ctx.ops.RxDone(usbdeluxe_host_hid_ctx.userp, interface_id, usbdeluxe_host_hid_ctx.report_data, rc_read);
				}
			}

			usbdeluxe_host_hid_ctx.task_state = 0;
			usbdeluxe_host_hid_ctx.current_interface_pos++;
		}
	}

}

#endif