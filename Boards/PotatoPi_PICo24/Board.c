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

#include "PICo24_Board.h"

#include <PICo24/Peripherals/UART/UART.h>
#include <PICo24/Peripherals/SPI/SPI.h>
#include <PICo24/Peripherals/I2C/I2C.h>
#include <PICo24/Peripherals/EXT_INT/EXT_INT.h>

// CONFIG3
#pragma config WPFP = WPFP255    //Write Protection Flash Page Segment Boundary->Highest Page (same as page 170)
#pragma config SOSCSEL = EC    //Secondary Oscillator Power Mode Select->External clock (SCLKI) or Digital I/O mode(
#pragma config WUTSEL = LEG    //Voltage Regulator Wake-up Time Select->Default regulator start-up time is used
#pragma config ALTPMP = ALPMPDIS    //Alternate PMP Pin Mapping->EPMP pins are in default location mode
#pragma config WPDIS = WPDIS    //Segment Write Protection Disable->Segmented code protection is disabled
#pragma config WPCFG = WPCFGDIS    //Write Protect Configuration Page Select->Last page (at the top of program memory) and Flash Configuration Words are not write-protected
#pragma config WPEND = WPENDMEM    //Segment Write Protection End Page Select->Protected code segment upper boundary is at the last page of program memory; the lower boundary is the code page specified by WPFP

// CONFIG2
#pragma config POSCMOD = XT    //Primary Oscillator Select->XT Oscillator mode is selected
#pragma config IOL1WAY = OFF    //IOLOCK One-Way Set Enable->The IOLOCK bit (OSCCON<6>) can be set once, provided the unlock sequence has been completed. Once set, the Peripheral Pin Select registers cannot be written to a second time.
#pragma config OSCIOFNC = ON    //OSCO Pin Configuration->OSCO/CLKO/RC15 functions as port I/O (RC15)
#pragma config FCKSM = CSECMD    //Clock Switching and Fail-Safe Clock Monitor->Clock switching is enabled, Fail-Safe Clock Monitor is disabled
#pragma config FNOSC = FRC    //Initial Oscillator Select->FRC
#pragma config PLL96MHZ = ON    //96MHz PLL Startup Select->96 MHz PLL is enabled automatically on start-up
#pragma config PLLDIV = DIV2    //96 MHz PLL Prescaler Select->Oscillator input is divided by 2 (8 MHz input)
#pragma config IESO = OFF    //Internal External Switchover->IESO mode (Two-Speed Start-up) is disabled

// CONFIG1
#pragma config WDTPS = PS32768    //Watchdog Timer Postscaler->1:32768
#pragma config FWPSA = PR128    //WDT Prescaler->Prescaler ratio of 1:128
#pragma config WINDIS = OFF    //Windowed WDT->Standard Watchdog Timer enabled,(Windowed-mode is disabled)
#pragma config FWDTEN = OFF    //Watchdog Timer->Watchdog Timer is disabled
#pragma config ICS = PGx3    //Emulator Pin Placement Select bits->Emulator functions are shared with PGEC3/PGED3
#pragma config GWRP = OFF    //General Segment Write Protect->Writes to program memory are allowed
#pragma config GCP = OFF    //General Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    //JTAG Port Enable->JTAG port is disabled

const uint32_t XTAL_FREQ = 32000000UL;
const uint32_t FCY = 32000000UL / 2;
const uint32_t FCY_DIV_1000 = 16000;
const uint16_t FCY_DIV_1000000 = 16;


const char PICo24_Board_Manufacturer[] = "SudoMaker";
const char PICo24_Board_Name[] = "PotatoPi PICo24";
const char PICo24_Board_ChipManufacturer[] = "Microchip";
const char PICo24_Board_Chip[] = "PIC24FJ256GB206";

void PICo24_Clock_Initialize() {
	// CPDIV 1:1; RCDIV FRC/2; DOZE 1:8; DOZEN disabled; ROI disabled;
	CLKDIV = 0x3100;
	// TUN Center frequency;
	OSCTUN = 0x00;
	// ADC1MD enabled; T3MD enabled; T4MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; SPI2MD enabled; SPI1MD enabled; T5MD enabled; I2C1MD enabled;
	PMD1 = 0x00;
	// OC5MD enabled; OC6MD enabled; OC7MD enabled; OC8MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; OC4MD enabled; IC6MD enabled; IC7MD enabled; IC5MD enabled; IC8MD enabled; IC4MD enabled; IC3MD enabled;
	PMD2 = 0x00;
	// I2C3MD enabled; PMPMD enabled; U3MD enabled; RTCCMD enabled; CMPMD enabled; CRCMD enabled; I2C2MD enabled;
	PMD3 = 0x00;
	// U4MD enabled; UPWMMD enabled; USB1MD enabled; CTMUMD enabled; REFOMD enabled; LVDMD enabled;
	PMD4 = 0x00;
	// IC9MD enabled; OC9MD enabled;
	PMD5 = 0x00;
	// SPI3MD enabled;
	PMD6 = 0x00;
	// CF no clock failure; NOSC PRIPLL; SOSCEN disabled; POSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete;
	__builtin_write_OSCCONH((uint8_t) (0x03));
	__builtin_write_OSCCONL((uint8_t) (0x01));
	// Wait for Clock switch to occur
	while (OSCCONbits.OSWEN != 0);
	while (OSCCONbits.LOCK != 1);
}


