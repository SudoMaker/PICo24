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

/** INCLUDES *******************************************************/
#include "../usb.h"
#include "usb_host.h"
#include "usb_host_hid.h"
#include "usb_host_cdc.h"
#include "../usb_deluxe.h"

#include <stdio.h>

#ifdef PICo24_Enable_Peripheral_USB_HOST

/****************************************************************************
  Function:
    bool USB_Host_EventHandler( uint8_t address, USB_EVENT event,
                void *data, uint32_t size )

  Summary:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.

  Description:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.  If the application is able to handle the event, it
    returns TRUE.  Otherwise, it returns FALSE.

  Precondition:
    None

  Parameters:
    uint8_t address    - Address of device where event occurred
    USB_EVENT event - Identifies the event that occured
    void *data      - Pointer to event-specific data
    uint32_t size      - Size of the event-specific data

  Return Values:
    TRUE    - The event was handled
    FALSE   - The event was not handled

  Remarks:
    The application may also implement an event handling routine if it
    requires knowledge of events.  To do so, it must implement a routine that
    matches this function signature and define the USB_HOST_APP_EVENT_HANDLER
    macro as the name of that function.
  ***************************************************************************/
bool USB_Host_EventHandler(uint8_t address, USB_EVENT event, void *data, uint32_t size )
{

	printf("USB host event: addr=%u, event=%u, data=%p, size=%lu\n", address, event, data, size);

	switch( (int)event )
	{
		/* Standard USB host events ******************************************/
		case EVENT_VBUS_REQUEST_POWER:
		case EVENT_VBUS_RELEASE_POWER:
		case EVENT_HUB_ATTACH:
		case EVENT_UNSUPPORTED_DEVICE:
		case EVENT_CANNOT_ENUMERATE:
		case EVENT_CLIENT_INIT_ERROR:
		case EVENT_OUT_OF_MEMORY:
		case EVENT_UNSPECIFIED_ERROR:
			return true;
			break;

			/* CDC Class Specific Events ******************************************/
		case EVENT_CDC_ATTACH:
		case EVENT_CDC_NONE:
		case EVENT_CDC_COMM_READ_DONE:
		case EVENT_CDC_COMM_WRITE_DONE:
		case EVENT_CDC_DATA_READ_DONE:
		case EVENT_CDC_DATA_WRITE_DONE:
		case EVENT_CDC_RESET:
		case EVENT_CDC_NAK_TIMEOUT:
			return true;
			break;

			/* HID Class Specific Events ******************************************/
		case EVENT_HID_ATTACH:
			printf("EVENT_HID_ATTACH\n");
			break;
		case EVENT_HID_DETACH:
			printf("EVENT_HID_DETACH\n");
			break;
		case EVENT_HID_RPT_DESC_PARSED:
			printf("EVENT_HID_RPT_DESC_PARSED\n");
			USBDeluxeHost_HID_ReportDescHandler();
			break;

		default:
			return false;
			break;
	}

	return true;
}

//void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
//{
//    USB_HostInterruptHandler();
//}

#endif