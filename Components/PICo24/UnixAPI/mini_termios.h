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

#include <stdint.h>

//	termios value		BRG value
#define B9600			0x1a0	// Error Rate: -0.08
#define B14400			0x115	// Error Rate: -0.08
#define B19200			0xcf	// Error Rate: 0.16
#define B38400			0x67	// Error Rate: 0.16
#define B57600			0x44	// Error Rate: 0.644
#define B115200			0x22	// Error Rate: -0.794
#define B230400			0x10	// Error Rate: 2.214
#define B460800			0x8	// Error Rate: -3.549
#define B1000000		0x3	// Error Rate: 0
#define B2000000		0x1	// Error Rate: 0

extern int cfsetspeed(int fd, uint16_t speed);
extern int cfgetspeed(int fd, uint16_t speed);