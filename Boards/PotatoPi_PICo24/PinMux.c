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
	LATC = 0x0000;
	LATD = 0x0000;
	LATE = 0x0000;
	LATF = 0x0000;
	LATG = 0x0000;

	/****************************************************************************
	 * Setting the GPIO Direction SFR(s)
	 ***************************************************************************/
	TRISB = 0xFF3C;
	TRISC = 0x7000;
	TRISD = 0x0FD7;
	TRISE = 0x00FF;
	TRISF = 0x0038;
	TRISG = 0x034C;

	/****************************************************************************
	 * Setting the Weak Pull Up and Weak Pull Down SFR(s)
	 ***************************************************************************/
	CNPD1 = 0x0000;
	CNPD2 = 0x0000;
	CNPD3 = 0x0000;
	CNPD4 = 0x0000;
	CNPD5 = 0x0000;
	CNPD6 = 0x0000;
	CNPU1 = 0x0000;
	CNPU2 = 0x0001;
	CNPU3 = 0x0000;
	CNPU4 = 0x0000;
	CNPU5 = 0x0000;
	CNPU6 = 0x0000;

	/****************************************************************************
	 * Setting the Open Drain SFR(s)
	 ***************************************************************************/
	ODCB = 0x0000;
	ODCC = 0x0000;
	ODCD = 0x0000;
	ODCE = 0x0000;
	ODCF = 0x0000;
	ODCG = 0x0000;

	/****************************************************************************
	 * Setting the Analog/Digital Configuration SFR(s)
	 ***************************************************************************/
	ANSB = 0xFF08;
	ANSC = 0x6000;
	ANSD = 0x0040;
	ANSF = 0x0000;
	ANSG = 0x0300;

	/****************************************************************************
	 * Set the PPS
	 ***************************************************************************/

	PICo24_PinConfig_Begin();

	// EXTINT3
	PICo24_PinConfig_Func(PIN_RF3_RP16, PINFUNC_EXTINT_3);

	// SPI1
	PICo24_PinConfig_Func(PIN_RD3_RP22, PINFUNC_SPI1_SCK_OUT);
	PICo24_PinConfig_Func(PIN_RD5_RP20, PINFUNC_SPI1_MOSI);
	PICo24_PinConfig_Func(PIN_RD4_RP25, PINFUNC_SPI1_MISO);

	// SPI2
	PICo24_PinConfig_Func(PIN_RB6_RP6, PINFUNC_SPI2_SCK_OUT);
	PICo24_PinConfig_Func(PIN_RB7_RP7, PINFUNC_SPI2_MOSI);
	PICo24_PinConfig_Func(PIN_RD8_RP2, PINFUNC_SPI2_MISO);

	// SPI3
	PICo24_PinConfig_Func(PIN_RB0_RP0, PINFUNC_SPI3_SCK_OUT);
	PICo24_PinConfig_Func(PIN_RB1_RP1, PINFUNC_SPI3_MOSI);
	PICo24_PinConfig_Func(PIN_RB2_RP13, PINFUNC_SPI3_MISO);

	// UART1
	PICo24_PinConfig_Func(PIN_RG6_RP21, PINFUNC_UART1_RX);
	PICo24_PinConfig_Func(PIN_RG7_RP26, PINFUNC_UART1_TX);

	PICo24_PinConfig_End();
}