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

#pragma once

#include <stdint.h>
#ifdef __XC16_VERSION__
#undef __XC16_VERSION__
#endif
#define __XC16_VERSION__ 1700
#include <xc.h>

extern uint8_t PICo24_Discard8;

#define LED_USER_0			LATAbits.LATA2
#define LED_USER_1			LATAbits.LATA3

#define PINFUNC_OUTPUT			0
#define PINFUNC_INPUT			1

#define PIN_RB0_RP0			0, RPOR0bits.RP0R
#define PIN_RB1_RP1			1, RPOR0bits.RP1R
#define PIN_RB2_RP13			13, RPOR6bits.RP13R
#define PIN_RB4_RP28			28, RPOR14bits.RP28R
#define PIN_RB5_RP18			18, RPOR9bits.RP18R
#define PIN_RB6_RP6			6, RPOR3bits.RP6R
#define PIN_RB7_RP7			7, RPOR3bits.RP7R
#define PIN_RB8_RP8			8, RPOR4bits.RP8R
#define PIN_RB9_RP9			9, RPOR4bits.RP9R
#define PIN_RB14_RP14			14, RPOR7bits.RP14R
#define PIN_RB15_RP29			29, RPOR14bits.RP29R

#define PIN_RD0_RP11			11, RPOR5bits.RP11R
#define PIN_RD1_RP24			24, RPOR12bits.RP24R
#define PIN_RD2_RP23			23, RPOR12bits.RP23R
#define PIN_RD3_RP22			22, RPOR11bits.RP22R
#define PIN_RD4_RP25			25, RPOR12bits.RP25R
#define PIN_RD5_RP20			20, RPOR10bits.RP20R
#define PIN_RD8_RP2			2, RPOR1bits.RP2R
#define PIN_RD9_RP4			4, RPOR2bits.RP4R
#define PIN_RD10_RP3			3, RPOR1bits.RP3R
#define PIN_RD11_RP12			12, RPOR6bits.RP12R

#define PIN_RF3_RP16			16, RPOR8bits.RP16R
#define PIN_RF4_RP10			10, RPOR5bits.RP10R
#define PIN_RF5_RP17			17, RPOR8bits.RP17R

#define PIN_RG6_RP21			21, RPOR10bits.RP21R
#define PIN_RG7_RP26			26, RPOR13bits.RP26R
#define PIN_RG8_RP19			19, RPOR9bits.RP19R
#define PIN_RG9_RP27			27, RPOR13bits.RP27R

#define PINFUNC_EXTINT_1		PINFUNC_INPUT, RPINR0bits.INT1R, 63
#define PINFUNC_EXTINT_2		PINFUNC_INPUT, RPINR1bits.INT2R, 63
#define PINFUNC_EXTINT_3		PINFUNC_INPUT, RPINR1bits.INT3R, 63
#define PINFUNC_EXTINT_4		PINFUNC_INPUT, RPINR2bits.INT4R, 63

#define PINFUNC_COMP_OUT1		PINFUNC_OUTPUT, PICo24_Discard8, 1
#define PINFUNC_COMP_OUT2		PINFUNC_OUTPUT, PICo24_Discard8, 2
#define PINFUNC_COMP_OUT3		PINFUNC_OUTPUT, PICo24_Discard8, 36

#define PINFUNC_IC_1			PINFUNC_INPUT, RPINR7bits.IC1R, 63
#define PINFUNC_IC_2			PINFUNC_INPUT, RPINR7bits.IC2R, 63
#define PINFUNC_IC_3			PINFUNC_INPUT, RPINR8bits.IC3R, 63
#define PINFUNC_IC_4			PINFUNC_INPUT, RPINR8bits.IC4R, 63
#define PINFUNC_IC_5			PINFUNC_INPUT, RPINR9bits.IC5R, 63
#define PINFUNC_IC_6			PINFUNC_INPUT, RPINR9bits.IC6R, 63
#define PINFUNC_IC_7			PINFUNC_INPUT, RPINR10bits.IC7R, 63
#define PINFUNC_IC_8			PINFUNC_INPUT, RPINR10bits.IC8R, 63
#define PINFUNC_IC_9			PINFUNC_INPUT, RPINR15bits.IC9R, 63

#define PINFUNC_OC_1			PINFUNC_OUTPUT, PICo24_Discard8, 18
#define PINFUNC_OC_2			PINFUNC_OUTPUT, PICo24_Discard8, 19
#define PINFUNC_OC_3			PINFUNC_OUTPUT, PICo24_Discard8, 20
#define PINFUNC_OC_4			PINFUNC_OUTPUT, PICo24_Discard8, 21
#define PINFUNC_OC_5			PINFUNC_OUTPUT, PICo24_Discard8, 22
#define PINFUNC_OC_6			PINFUNC_OUTPUT, PICo24_Discard8, 23
#define PINFUNC_OC_7			PINFUNC_OUTPUT, PICo24_Discard8, 24
#define PINFUNC_OC_8			PINFUNC_OUTPUT, PICo24_Discard8, 25
#define PINFUNC_OC_9			PINFUNC_OUTPUT, PICo24_Discard8, 35

#define PINFUNC_OC_FAULTA		PINFUNC_INPUT, RPINR11bits.OCFA, 63
#define PINFUNC_OC_FAULTB		PINFUNC_INPUT, RPINR11bits.OCFB, 63

#define PINFUNC_SPI1_SCK_IN		PINFUNC_INPUT, RPINR20bits.SCK1R, 63
#define PINFUNC_SPI1_MISO		PINFUNC_INPUT, RPINR20bits.SDI1R, 63
#define PINFUNC_SPI1_CS_IN		PINFUNC_INPUT, RPINR21bits.SS1R, 63
#define PINFUNC_SPI1_SCK_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 8
#define PINFUNC_SPI1_CS_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 9
#define PINFUNC_SPI1_MOSI		PINFUNC_OUTPUT, PICo24_Discard8, 7

#define PINFUNC_SPI2_SCK_IN		PINFUNC_INPUT, RPINR22bits.SCK2R, 63
#define PINFUNC_SPI2_MISO		PINFUNC_INPUT, RPINR22bits.SDI2R, 63
#define PINFUNC_SPI2_CS_IN		PINFUNC_INPUT, RPINR23bits.SS2R, 63
#define PINFUNC_SPI2_SCK_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 11
#define PINFUNC_SPI2_CS_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 12
#define PINFUNC_SPI2_MOSI		PINFUNC_OUTPUT, PICo24_Discard8, 10

#define PINFUNC_SPI3_SCK_IN		PINFUNC_INPUT, RPINR28bits.SCK3R, 63
#define PINFUNC_SPI3_MISO		PINFUNC_INPUT, RPINR28bits.SDI3R, 63
#define PINFUNC_SPI3_CS_IN		PINFUNC_INPUT, RPINR29bits.SS3R, 63
#define PINFUNC_SPI3_SCK_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 33
#define PINFUNC_SPI3_CS_OUT		PINFUNC_OUTPUT, PICo24_Discard8, 34
#define PINFUNC_SPI3_MOSI		PINFUNC_OUTPUT, PICo24_Discard8, 32

#define PINFUNC_TIM2_EXTCLK		PINFUNC_INPUT, RPINR3bits.T2CKR, 63
#define PINFUNC_TIM3_EXTCLK		PINFUNC_INPUT, RPINR3bits.T3CKR, 63
#define PINFUNC_TIM4_EXTCLK		PINFUNC_INPUT, RPINR4bits.T4CKR, 63
#define PINFUNC_TIM5_EXTCLK		PINFUNC_INPUT, RPINR4bits.T5CKR, 63

#define PINFUNC_UART1_CTS		PINFUNC_INPUT, RPINR18bits.U1CTSR, 63
#define PINFUNC_UART1_RX		PINFUNC_INPUT, RPINR18bits.U1RXR, 63
#define PINFUNC_UART1_RTS		PINFUNC_OUTPUT, PICo24_Discard8, 4
#define PINFUNC_UART1_TX		PINFUNC_OUTPUT, PICo24_Discard8, 3

#define PINFUNC_UART2_CTS		PINFUNC_INPUT, RPINR19bits.U2CTSR, 63
#define PINFUNC_UART2_RX		PINFUNC_INPUT, RPINR19bits.U2RXR, 63
#define PINFUNC_UART2_RTS		PINFUNC_OUTPUT, PICo24_Discard8, 6
#define PINFUNC_UART2_TX		PINFUNC_OUTPUT, PICo24_Discard8, 5



#define PICo24_PinFuncRaw(RPx, RPOR_REG, TAG, RPINR_REG, OUT_FUNC)	if (TAG == PINFUNC_OUTPUT) RPOR_REG = OUT_FUNC; else if (TAG == PINFUNC_INPUT) RPINR_REG = RPx;
#define PICo24_PinConfig_Func(PIN, FUNC)					PICo24_PinFuncRaw(PIN, FUNC)

extern void PICo24_PinConfig_Begin();
extern void PICo24_PinConfig_End();
extern void PICo24_PinConfig_UseDefault();