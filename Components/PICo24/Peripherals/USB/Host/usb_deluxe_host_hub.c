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

#include "usb_deluxe_host_hub.h"
#include "usb_host_local.h"
#include <stdio.h>
#include <PICo24/Core/Delay.h>

// After reading https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/usbspec/ns-usbspec-_usb_hub_descriptor ,
// Finally I know these typedef struct _FOO_BAR, FOO_BAR, *PFOO_BAR nonsense are started by Micro$oft. That's ugly as hell.
// And brainless people keep copying these without thinking.

// And I should blame MCHP once more here: PIC24s don't deserve USB hub support?

// Also read this post: https://www.microchip.com/forums/FindPost/523499

typedef struct {
	uint8_t bDescriptorLength;
	uint8_t  bDescriptorType;
	uint8_t  bNumberOfPorts;
	uint16_t wHubCharacteristics;
	uint8_t  bPowerOnToPowerGood;
	uint8_t  bHubControlCurrent;
	uint8_t  bRemoveAndPowerMask;
} __attribute__((packed)) USBDeluxeHost_Hub_Descriptor;

typedef struct {
	USBDeluxeHost_Hub_Descriptor descriptor;
	USB_ENDPOINT_INFO *endpoint_info;
	Vector slave_device_addresses;
	Vector change_notif_buffer;
	Vector port_status;

	uint16_t pending_port_set_features;
	uint16_t pending_port_clr_features;
	uint16_t pending_port_c_features;

	uint8_t address;
	uint8_t state;
	uint8_t last_state;
	uint8_t current_port;
	uint8_t current_req;
	uint8_t current_feature;

} USBDeluxeHost_HubContext;

Vector hubs = {
	.size = 0,
	.data = NULL,
	.element_size = sizeof(USBDeluxeHost_HubContext)
};

// USB 2.0 Spec 11.24.2 Class-specific Requests, usb_20.pdf page 421
enum USBDeluxeHost_Hub_Request {
	HUB_REQUEST_GET_STATUS		= 0,
	HUB_REQUEST_CLEAR_FEATURE	= 1,
	HUB_REQUEST_SET_FEATURE		= 3,
	HUB_REQUEST_GET_DESCRIPTOR	= 6,
	HUB_REQUEST_SET_DESCRIPTOR	= 7,
	HUB_REQUEST_CLEAR_TT_BUFFER	= 8,
	HUB_REQUEST_RESET_TT		= 9,
	HUB_REQUEST_GET_TT_STATE	= 10,
	HUB_REQUEST_STOP_TT		= 11,
};

enum USBDeluxeHost_Hub_Feature {
	HUB_FEATURE_C_HUB_LOCAL_POWER	= 0,
	HUB_FEATURE_C_HUB_OVER_CURRENT	= 1,
	HUB_FEATURE_PORT_CONNECTION	= 0,
	HUB_FEATURE_PORT_ENABLE		= 1,
	HUB_FEATURE_PORT_SUSPEND	= 2,
	HUB_FEATURE_PORT_OVER_CURRENT	= 3,
	HUB_FEATURE_PORT_RESET		= 4,
	HUB_FEATURE_PORT_POWER		= 8,
	HUB_FEATURE_PORT_LOW_SPEED	= 9,
	HUB_FEATURE_C_PORT_CONNECTION	= 16,
	HUB_FEATURE_C_PORT_ENABLE	= 17,
	HUB_FEATURE_C_PORT_SUSPEND	= 18,
	HUB_FEATURE_C_PORT_OVER_CURRENT	= 19,
	HUB_FEATURE_C_PORT_RESET	= 20,
	HUB_FEATURE_C_PORT_TEST		= 21,
	HUB_FEATURE_C_PORT_INDICATOR	= 22,
};

enum USBDeluxeHost_Hub_PortStatusFlag {
	// USB 2.0 Spec 11.24.2.7.1 Port Status Bits, usb_20.pdf page 427
	HUB_PORTSTAT_PORT_CONNECTION	= (1U << 0),
	HUB_PORTSTAT_PORT_ENABLE	= (1U << 1),
	HUB_PORTSTAT_PORT_SUSPEND	= (1U << 2),
	HUB_PORTSTAT_PORT_OVER_CURRENT	= (1U << 3),
	HUB_PORTSTAT_PORT_RESET		= (1U << 4),
	HUB_PORTSTAT_PORT_POWER		= (1U << 8),
	HUB_PORTSTAT_PORT_LOW_SPEED	= (1U << 9),
	HUB_PORTSTAT_PORT_HIGH_SPEED	= (1U << 10),
	HUB_PORTSTAT_PORT_TEST		= (1U << 11),
	HUB_PORTSTAT_PORT_INDICATOR	= (1U << 12),

	// USB 2.0 Spec 11.24.2.7.2 Port Status Change Bits, usb_20.pdf page 431
	HUB_PORTSTAT_C_PORT_CONNECTION		= (1U << 0),
	HUB_PORTSTAT_C_PORT_ENABLE		= (1U << 1),
	HUB_PORTSTAT_C_PORT_SUSPEND		= (1U << 2),
	HUB_PORTSTAT_C_PORT_OVER_CURRENT	= (1U << 3),
	HUB_PORTSTAT_C_PORT_RESET		= (1U << 4),
};

enum USBDeluxeHost_Hub_State {
	HUB_STATE_NONE = 0,

	HUB_STATE_READ_STATUS_CHANGE,
	HUB_STATE_READ_STATUS_CHANGE_STARTED,

	HUB_STATE_GET_PORT_STATUS,
	HUB_STATE_GET_PORT_STATUS_STARTED,

	HUB_STATE_PARSE_PORT_STATUS,

	HUB_STATE_SET_PORT_FEATURE,
	HUB_STATE_SET_PORT_FEATURE_STARTED,

};

static uint8_t hub_driver_id = 0; // This is stupid

bool USBHostHubInitialize(uint8_t address, uint32_t flags, uint8_t clientDriverID) {
	printf("USBHostHubInitialize: addr=%u\n", address);

	USBDeluxeHost_HubContext *new_hub = Vector_EmplaceBack(&hubs);
	memset(new_hub, 0, sizeof(USBDeluxeHost_HubContext));

	new_hub->address = address;
	new_hub->endpoint_info = USBHostGetDeviceInfoBySlotIndex(address)->pInterfaceList->pCurrentSetting->pEndpointList;
	Vector_Init(&new_hub->slave_device_addresses, sizeof(uint8_t));
	Vector_Init(&new_hub->port_status, sizeof(uint32_t));
	Vector_Init(&new_hub->change_notif_buffer, sizeof(uint8_t));
	Vector_Resize(&new_hub->change_notif_buffer, new_hub->endpoint_info->wMaxPacketSize);
	new_hub->state = HUB_STATE_READ_STATUS_CHANGE;

	printf("USBHostHubInitialize: wMaxPacketSize=%u\n", new_hub->endpoint_info->wMaxPacketSize);


	uint8_t rc = USBHostIssueDeviceRequest(
		address,
		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE,
		HUB_REQUEST_GET_DESCRIPTOR,
		0x2900,
		0,
		sizeof(USBDeluxeHost_Hub_Descriptor),
		(uint8_t *)&new_hub->descriptor,
		USB_DEVICE_REQUEST_GET,
		clientDriverID
	);

	if (rc == USB_SUCCESS) {
		uint8_t ec_;
		uint32_t len_;
		while (!USBHostTransferIsComplete(address, 0, &ec_, &len_));
	} else {
		return false;
	}

	printf("HUB descriptor dump:\n");
	hexdump((const uint8_t *) &new_hub->descriptor, sizeof(USBDeluxeHost_Hub_Descriptor));

	printf("This HUB has %u ports\n", new_hub->descriptor.bNumberOfPorts);
	Vector_Resize(&new_hub->port_status, new_hub->descriptor.bNumberOfPorts);
	Vector_Resize(&new_hub->slave_device_addresses, new_hub->descriptor.bNumberOfPorts);
	memset(new_hub->slave_device_addresses.data, 0, new_hub->descriptor.bNumberOfPorts);

	for (size_t i=1; i<=new_hub->descriptor.bNumberOfPorts; i++) {
		uint8_t rc2 = USBHostIssueDeviceRequest(
			address,
			USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER,
			HUB_REQUEST_SET_FEATURE,
			HUB_FEATURE_PORT_POWER,
			i,
			0,
			NULL,
			USB_DEVICE_REQUEST_SET,
			clientDriverID
		);

		if (rc2 == USB_SUCCESS) {
			uint8_t ec_;
			uint32_t len_;
			while (!USBHostTransferIsComplete(address, 0, &ec_, &len_));
		} else {
			return false;
		}

		printf("port %u power on\n", i);
	}

	hub_driver_id = clientDriverID;

	return true;
}

bool USBHostHubEventHandler(uint8_t address, USB_EVENT event, void *data, uint32_t size) {
	printf("USBHostHubEventHandler: addr=%u, event=%d, data=%p, size=%lu\n", address, event, data, size);

	if (event == EVENT_DETACH) {
		Vector_ForEach(it, hubs) {
			USBDeluxeHost_HubContext *this_hub = it;

			if (this_hub->address == address) {
				Vector_ForEach(it2, this_hub->slave_device_addresses) {
					uint8_t *slave_addr = it2;
					if (*slave_addr) {
						printf("USBHostHubEventHandler: DETACH: shutdown port %ld slave: addr=%u\n",
						       Vector_DistanceFromBegin(&this_hub->slave_device_addresses, it2), *slave_addr);
						USBHostShutdown(*slave_addr);
					}
				}


				Vector_Clear(&this_hub->slave_device_addresses);
				Vector_Clear(&this_hub->change_notif_buffer);
				Vector_Clear(&this_hub->port_status);
				Vector_Erase(&hubs, this_hub);
				printf("USBHostHubEventHandler: DETACH: HUB 0x%p removed\n", this_hub);
				break;
			}
		}
	} else if (event == EVENT_TRANSFER) {
		hexdump(data, size);
	}

	return true;
}

void USBHostHubTasks() {
	Vector_ForEach(it, hubs) {
		USBDeluxeHost_HubContext *this_hub = it;

		if (this_hub->state != this_hub->last_state) {
			printf("USBHostHubTasks: hub=0x%04x, state=%u\n", this_hub, this_hub->state);
			this_hub->last_state = this_hub->state;
		}

		switch (this_hub->state) {
			case HUB_STATE_READ_STATUS_CHANGE: {
				if (USBHostRead(this_hub->address, this_hub->endpoint_info->bEndpointAddress, this_hub->change_notif_buffer.data, this_hub->change_notif_buffer.size) == USB_SUCCESS) {
					this_hub->state = HUB_STATE_READ_STATUS_CHANGE_STARTED;
				} else {
					printf("READ_STATUS_CHANGE: error1\n");
					USBHostClearEndpointErrors(this_hub->address, this_hub->endpoint_info->bEndpointAddress);
				}
				break;
			}
			case HUB_STATE_READ_STATUS_CHANGE_STARTED: {
				uint8_t ec_;
				uint32_t len_;

				if (USBHostTransferIsComplete(this_hub->address, this_hub->endpoint_info->bEndpointAddress, &ec_, &len_)) {
					if (ec_ == USB_SUCCESS) {
						if (len_ && this_hub->change_notif_buffer.size) {
							printf("READ_STATUS_CHANGE: len=%lu\n", len_);
							hexdump(this_hub->change_notif_buffer.data, len_);

							if (((uint8_t *)this_hub->change_notif_buffer.data)) {
								this_hub->state = HUB_STATE_GET_PORT_STATUS;
								break;
							}
						}
					} else {
						printf("READ_STATUS_CHANGE: error2\n");
						USBHostClearEndpointErrors(this_hub->address, this_hub->endpoint_info->bEndpointAddress);
					}

					this_hub->state = HUB_STATE_READ_STATUS_CHANGE;
				}
				break;
			}
			case HUB_STATE_GET_PORT_STATUS: {
				if (!this_hub->current_port) {
					printf("GET_PORT_STATUS: no current port, checking\n");

					uint8_t *pchanges = this_hub->change_notif_buffer.data;

					for (size_t byte_pos = 0; byte_pos < this_hub->change_notif_buffer.size; byte_pos += 1) {
						for (uint8_t bit_pos = 1; bit_pos < 8; bit_pos += 1) {
							printf("GET_PORT_STATUS: checking port %u\n", bit_pos);

							uint8_t mask = (0x1 << bit_pos);
							if (*(pchanges + byte_pos) & mask) {
								printf("GET_PORT_STATUS: got port %u\n", bit_pos);
								this_hub->current_port = bit_pos + byte_pos * 8;
								*(pchanges + byte_pos) &= ~mask;
								break;
							}
						}

						if (this_hub->current_port) {
							break;
						}
					}
				} else {
					printf("GET_PORT_STATUS: Using last port %u\n", this_hub->current_port);
				}

				if (this_hub->current_port) {
					if (USBHostIssueDeviceRequest(
						this_hub->address,
						USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_OTHER,
						HUB_REQUEST_GET_STATUS,
						0,
						this_hub->current_port,
						4,
						Vector_At(&this_hub->port_status, this_hub->current_port - 1),
						USB_DEVICE_REQUEST_GET,
						hub_driver_id
					) == USB_SUCCESS) {
						this_hub->state = HUB_STATE_GET_PORT_STATUS_STARTED;
					}
				} else {
					this_hub->state = HUB_STATE_READ_STATUS_CHANGE;
				}
				break;
			}
			case HUB_STATE_GET_PORT_STATUS_STARTED: {
				uint8_t ec_;
				uint32_t len_;
				if (USBHostTransferIsComplete(this_hub->address, 0, &ec_, &len_)) {
					this_hub->state = HUB_STATE_PARSE_PORT_STATUS;
				} else {
					this_hub->state = HUB_STATE_GET_PORT_STATUS;
					USBHostClearEndpointErrors(this_hub->address, 0);
				}
				break;
			}
			case HUB_STATE_PARSE_PORT_STATUS: {
				uint16_t *wPortData = Vector_At(&this_hub->port_status, this_hub->current_port - 1);
				uint16_t wPortStatus = wPortData[0];
				uint16_t wPortChange = wPortData[1];

				this_hub->pending_port_c_features = wPortChange;

				printf("PARSE_PORT_STATUS: wPortStatus=0x%04x, wPortChange=0x%04x\n", wPortStatus, wPortChange);

				bool attached = false;

				if (wPortChange) {
					if (wPortChange & HUB_PORTSTAT_C_PORT_CONNECTION) {
						if (wPortStatus & HUB_PORTSTAT_PORT_CONNECTION) {
							if (wPortStatus & HUB_PORTSTAT_PORT_ENABLE) { // Attach
								printf("PARSE_PORT_STATUS: Attached! 1\n");
								attached = true;
//							this_hub->pending_port_set_features |= ((1U << HUB_FEATURE_PORT_RESET));
							} else { // Connect
								printf("PARSE_PORT_STATUS: Connected!\n");
								this_hub->pending_port_set_features |= ((1U << HUB_FEATURE_PORT_RESET));
							}
						} else { // Detach
							printf("PARSE_PORT_STATUS: Detached!\n");
							uint8_t *ppaddr = Vector_At(&this_hub->slave_device_addresses, this_hub->current_port);
							printf("Shutting down address %u\n", *ppaddr);
							USBHostShutdown(*ppaddr);
							USBHostConfigureDeviceSlot(*ppaddr, 0, 0);

						}
					} else if (wPortChange & HUB_PORTSTAT_C_PORT_RESET) {
						if (wPortStatus & HUB_PORTSTAT_PORT_CONNECTION) {
							if (wPortStatus & HUB_PORTSTAT_PORT_ENABLE) { // Attach
								printf("PARSE_PORT_STATUS: Attached! 2\n");
								attached = true;
							}
						}
					}
				} else {
					this_hub->current_port = 0;
					printf("PARSE_PORT_STATUS: Nothing changed, resetting current_port and going to step 1!\n");
					this_hub->state = HUB_STATE_READ_STATUS_CHANGE;
					break;
				}

				if (attached) {
					uint8_t new_device_addr = USBHostGetUnusedDeviceSlot();
					printf("New device addr for port %u: %u\n", this_hub->current_port, new_device_addr);

					uint8_t *ppaddr = Vector_At(&this_hub->slave_device_addresses, this_hub->current_port);
					*ppaddr = new_device_addr;
					USBHostInit(new_device_addr);
					bool low_speed = 0;
					if (wPortStatus & HUB_PORTSTAT_PORT_LOW_SPEED)
						low_speed = 1;
					USBHostConfigureDeviceSlot(new_device_addr, 1, low_speed);
				}

				this_hub->state = HUB_STATE_SET_PORT_FEATURE;

				break;
			}
			case HUB_STATE_SET_PORT_FEATURE: {
				bool has_req = false;

				if (this_hub->pending_port_c_features) {
					for (size_t i=0; i<16; i++) {
						if (this_hub->pending_port_c_features & (1U << i)) {
							this_hub->current_feature = i + 16;
							this_hub->current_req = HUB_REQUEST_CLEAR_FEATURE;
							has_req = true;
							this_hub->pending_port_c_features &= ~(1U << i);
							break;
						}
					}
				}

				if (!has_req) {
					if (this_hub->pending_port_set_features) {
						for (size_t i=0; i<16; i++) {
							if (this_hub->pending_port_set_features & (1U << i)) {
								this_hub->current_feature = i;
								this_hub->current_req = HUB_REQUEST_SET_FEATURE;
								has_req = true;
								this_hub->pending_port_set_features &= ~(1U << i);
								break;
							}
						}
					}
				}

				if (!has_req) {
					if (this_hub->pending_port_clr_features) {
						for (size_t i=0; i<16; i++) {
							if (this_hub->pending_port_clr_features & (1U << i)) {
								this_hub->current_feature = i;
								this_hub->current_req = HUB_REQUEST_CLEAR_FEATURE;
								has_req = true;
								this_hub->pending_port_clr_features &= ~(1U << i);
								break;
							}
						}
					}
				}

				if (!has_req) {
					printf("SET_PORT_FEATURE: noting to do\n");
					this_hub->state = HUB_STATE_GET_PORT_STATUS;
					break;
				}

				printf("SET_PORT_FEATURE: req=%u, feat=%u, port=%u\n", this_hub->current_req, this_hub->current_feature, this_hub->current_port);

				if (USBHostIssueDeviceRequest(
					this_hub->address,
					USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER,
					this_hub->current_req,
					this_hub->current_feature,
					this_hub->current_port,
					0,
					NULL,
					USB_DEVICE_REQUEST_SET,
					hub_driver_id) == USB_SUCCESS) {

					this_hub->state = HUB_STATE_SET_PORT_FEATURE_STARTED;

				} else {
					this_hub->state = HUB_STATE_GET_PORT_STATUS;
				}
			}
				break;
			case HUB_STATE_SET_PORT_FEATURE_STARTED: {
				uint8_t ec_;
				uint32_t len_;
				if (USBHostTransferIsComplete(this_hub->address, 0, &ec_, &len_)) {
					printf("SET_PORT_FEATURE done\n");
					if (this_hub->current_feature == HUB_FEATURE_PORT_RESET) {
						Delay_Milliseconds(100);
					}
					this_hub->state = HUB_STATE_SET_PORT_FEATURE;

				} else {
					USBHostClearEndpointErrors(this_hub->address, 0);
					printf("SET_PORT_FEATURE error\n");
					printf("RETRY SET_PORT_FEATURE: req=%u, feat=%u, port=%u\n", this_hub->current_req, this_hub->current_feature, this_hub->current_port);

					if (USBHostIssueDeviceRequest(
						this_hub->address,
						USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER,
						this_hub->current_req,
						this_hub->current_feature,
						this_hub->current_port,
						0,
						NULL,
						USB_DEVICE_REQUEST_SET,
						hub_driver_id) == USB_SUCCESS) {

						this_hub->state = HUB_STATE_SET_PORT_FEATURE_STARTED;

					} else {
						printf("RETRY SET_PORT_FEATURE failed\n");

//						this_hub->state = HUB_STATE_GET_PORT_STATUS;
					}
				}


				break;
			}
		}
	}
}

void USBDeluxe_HostDriver_Enable_Hub() {
	uint16_t host_hub = USBDeluxe_Host_AddDriver(USB_FUNC_HUB, USBHostHubInitialize, USBHostHubEventHandler, NULL, 0);

	USBDeluxe_Host_AddDeviceClass(host_hub, 0, 9, 0, 0);
}