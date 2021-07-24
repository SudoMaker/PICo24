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

#pragma once

/* Functional Descriptor Structure - See CDC Specification 1.2 for details */

/* Header Functional Descriptor */
typedef struct _USB_CDC_HEADER_FN_DSC
{
	uint8_t bFNLength;         // Size of this functional descriptor, in uint8_ts.
	uint8_t bDscType;          // CS_INTERFACE
	uint8_t bDscSubType;       // Header. This is defined in [USBCDC1.2], which defines this as a header.
	uint8_t bcdCDC[2];         // USB Class Definitions for Communications Devices Specification release number in binary-coded decimal.
} USB_CDC_HEADER_FN_DSC;

/* Abstract Control Management Functional Descriptor */
typedef struct _USB_CDC_ACM_FN_DSC
{
	uint8_t bFNLength;         // Size of this functional descriptor, in uint8_ts.
	uint8_t bDscType;          // CS_INTERFACE
	uint8_t bDscSubType;       // Abstract Control Management functional descriptor subtype as defined in [USBCDC1.2].
	uint8_t bmCapabilities;    // The capabilities that this configuration supports. (A bit value of zero means that the request is not supported.)
} USB_CDC_ACM_FN_DSC;

/* Union Functional Descriptor */
typedef struct _USB_CDC_UNION_FN_DSC
{
	uint8_t bFNLength;        // Size of this functional descriptor, in uint8_ts.
	uint8_t bDscType;         // CS_INTERFACE
	uint8_t bDscSubType;      // Union Descriptor Functional Descriptor subtype as defined in [USBCDC1.2].
	uint8_t bMasterIntf;      // Interface number of the control (Communications Class) interface
	uint8_t bSaveIntf0;       // Interface number of the subordinate (Data Class) interface
} USB_CDC_UNION_FN_DSC;

/* Call Management Functional Descriptor */
typedef struct _USB_CDC_CALL_MGT_FN_DSC
{
	uint8_t bFNLength;         // Size of this functional descriptor, in uint8_ts.
	uint8_t bDscType;          // CS_INTERFACE
	uint8_t bDscSubType;       // Call Management functional descriptor subtype, as defined in [USBCDC1.2].
	uint8_t bmCapabilities;    // The capabilities that this configuration supports:
	uint8_t bDataInterface;    // Interface number of Data Class interface optionally used for call management.
} USB_CDC_CALL_MGT_FN_DSC;

