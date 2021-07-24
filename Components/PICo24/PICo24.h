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

#include <PICo24/Core/Core.h>
#include <PICo24/Core/IDESupport.h>
#include <PICo24/Core/Delay.h>

#include <PICo24/Library/Variant.h>
#include <PICo24/Library/Vector.h>
#include <PICo24/Library/Vector_EDS.h>
#include <PICo24/Library/Set.h>
#include <PICo24/Library/SafeMalloc.h>
#include <PICo24/Library/DebugTools.h>


#include <PICo24/Peripherals/SPI/SPI.h>
#include <PICo24/Peripherals/UART/UART.h>
#include <PICo24/Peripherals/I2C/I2C.h>
#include <PICo24/Peripherals/EXT_INT/EXT_INT.h>
#include <PICo24/Peripherals/Timer/Timer.h>
#include <PICo24/Peripherals/USB/usb_deluxe.h>

#include <PICo24/UnixAPI/mini_unistd.h>
#include <PICo24/UnixAPI/mini_stdio.h>
#include <PICo24/UnixAPI/mini_termios.h>

#include <PICo24_Board.h>

#include <ScratchLibc/ScratchLibc.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <FreeRTOS/queue.h>
#include <FreeRTOS/croutine.h>