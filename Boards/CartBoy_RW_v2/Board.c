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

#include "PICo24_Board.h"


// CONFIG3
#pragma config WPFP = WPFP511    //Write Protection Flash Page Segment Boundary->Highest Page (same as page 170)
#pragma config WPDIS = WPDIS    //Segment Write Protection Disable bit->Segmented code protection disabled
#pragma config WPCFG = WPCFGDIS    //Configuration Word Code Page Protection Select bit->Last page(at the top of program memory) and Flash configuration words are not protected
#pragma config WPEND = WPENDMEM    //Segment Write Protection End Page Select bit->Write Protect from WPFP to the last page of memory

// CONFIG2
#pragma config POSCMOD = XT    //Primary Oscillator Select->XT oscillator mode selected
#pragma config DISUVREG = OFF    //Internal USB 3.3V Regulator Disable bit->Regulator is disabled
#pragma config IOL1WAY = OFF    //IOLOCK One-Way Set Enable bit->Write RP Registers Once
#pragma config OSCIOFNC = ON    //Primary Oscillator Output Function->OSCO functions as port I/O (RC15)
#pragma config FCKSM = CSECMD    //Clock Switching and Monitor->Clock switching is enabled, Fail-safe Clock Monitor is disabled
#pragma config FNOSC = FRC    //Oscillator Select->FRC
#pragma config PLL_96MHZ = ON    //96MHz PLL Disable->Enabled
#pragma config PLLDIV = DIV2    //USB 96 MHz PLL Prescaler Select bits->Oscillator input divided by 2 (8MHz input)
#pragma config IESO = OFF    //Internal External Switch Over Mode->IESO mode (Two-speed start-up)disabled

// CONFIG1
#pragma config WDTPS = PS32768    //Watchdog Timer Postscaler->1:32768
#pragma config FWPSA = PR128    //WDT Prescaler->Prescaler ratio of 1:128
#pragma config WINDIS = OFF    //Watchdog Timer Window->Standard Watchdog Timer enabled,(Windowed-mode is disabled)
#pragma config FWDTEN = OFF    //Watchdog Timer Enable->Watchdog Timer is disabled
#pragma config ICS = PGx2    //Comm Channel Select->Emulator functions are shared with PGEC1/PGED1
#pragma config BKBUG = OFF    //Background Debug->Device resets into Operational mode
#pragma config GWRP = OFF    //General Code Segment Write Protect->Writes to program memory are allowed
#pragma config GCP = OFF    //General Code Segment Code Protect->Code protection is disabled
#pragma config JTAGEN = OFF    //JTAG Port Enable->JTAG port is disabled

const uint32_t XTAL_FREQ = 32000000UL;
const uint32_t FCY = 32000000UL / 2;
const uint32_t FCY_DIV_1000 = 16000;
const uint16_t FCY_DIV_1000000 = 16;


const char PICo24_Board_Manufacturer[] = "SudoMaker";
const char PICo24_Board_Name[] = "CartBoy RW v2";
const char PICo24_Board_ChipManufacturer[] = "Microchip";
const char PICo24_Board_Chip[] = "PIC24FJ256GB108";

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
