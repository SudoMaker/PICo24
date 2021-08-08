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

#include "PinMux.h"

void PICo24_PinConfig_Begin() {
	__builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
}

void PICo24_PinConfig_End() {
	__builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
}

void PICo24_PinConfig_UseDefault() {
	/****************************************************************************
	 * Setting the Output Latch SFR(s)
	 ***************************************************************************/
	LATB = 0x0000;

	/****************************************************************************
	 * Setting the GPIO Direction SFR(s)
	 ***************************************************************************/
	TRISB = 0xFF3C;

	/****************************************************************************
	 * Setting the Weak Pull Up and Weak Pull Down SFR(s)
	 ***************************************************************************/
	CNPU1 = 0x0000;
	CNPU2 = 0x0000;

	/****************************************************************************
	 * Setting the Open Drain SFR(s)
	 ***************************************************************************/
	ODCB = 0x0000;

	/****************************************************************************
	 * Set the PPS
	 ***************************************************************************/

	PICo24_PinConfig_Begin();

	PICo24_PinConfig_End();
}