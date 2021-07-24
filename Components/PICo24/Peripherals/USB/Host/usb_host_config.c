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

// Created by the Microchip USBConfig Utility, Version 1.0.4.0, 4/25/2008, 17:05:22

#include "../usb.h"
#include "usb_host.h"
#include "usb_host_config.h"
#include "usb_host_hid.h"
#include "usb_host_hid_config.h"
#include "usb_host_cdc.h"
#include "usb_host_cdc_config.h"

// *****************************************************************************
// Client Driver Function Pointer Table for the USB Embedded Host foundation
// *****************************************************************************

Vector usbClientDrvTable = {
	.size = 0,
	.element_size = sizeof(CLIENT_DRIVER_TABLE),
	.data = NULL
};

// *****************************************************************************
// USB Embedded Host Targeted Peripheral List (TPL)
// *****************************************************************************

Vector usbTPL = {
	.size = 0,
	.element_size = sizeof(USB_TPL),
	.data = NULL
};

//USB_TPL usbTPL[] =
//{
//    { INIT_CL_SC_P( 2ul, 2ul, 2ul ), 0, 0, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 0x0Aul, 0ul, 0ul ), 0, 0, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 3ul, 1ul, 1ul ), 0, 1, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 3ul, 0ul, 1ul ), 0, 1, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 3ul, 1ul, 2ul ), 0, 1, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 3ul, 0ul, 2ul ), 0, 1, {TPL_CLASS_DRV} },
//    { INIT_CL_SC_P( 3ul, 0ul, 0ul ), 0, 1, {TPL_CLASS_DRV} }
//};

