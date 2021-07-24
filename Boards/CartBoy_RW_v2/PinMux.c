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
	LATA = 0x0000;
	LATB = 0x0000;
	LATC = 0x0000;
	LATD = 0x0000;
	LATE = 0x0000;
	LATF = 0x0000;
	LATG = 0x0000;

	/****************************************************************************
	 * Setting the GPIO Direction SFR(s)
	 ***************************************************************************/
	TRISA = 0xC600;
	TRISB = 0xC7FF; // 11~13: OUTPUT
	TRISC = 0x700A;
	TRISD = 0x0000;
	TRISE = 0x0300;
	TRISF = 0x0120;
	TRISG = 0x03CF;

	/****************************************************************************
	 * Setting the Weak Pull Up and Weak Pull Down SFR(s)
	 ***************************************************************************/
	CNPD1 = 0x0000;
	CNPD2 = 0x0000;
	CNPD3 = 0x0000;
	CNPD4 = 0x0000;
	CNPD5 = 0x0000;

	CNPU1 = 0x0000;
	CNPU2 = 0x0000;
	CNPU3 = 0x0000;
	CNPU4 = 0x0000;
	CNPU5 = 0x0000;

	/****************************************************************************
	 * Setting the Open Drain SFR(s)
	 ***************************************************************************/
	ODCA = 0x0000;
	ODCB = 0x0000;
	ODCC = 0x0000;
	ODCD = 0x0000;
	ODCE = 0x0000;
	ODCF = 0x0000;
	ODCG = 0x0000;

	/****************************************************************************
	 * Setting the Analog/Digital Configuration SFR(s)
	 ***************************************************************************/
	AD1PCFGH = 0x0000;
	AD1PCFGL = 0x7003;
}