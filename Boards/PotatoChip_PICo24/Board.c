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

#include <PICo24/Peripherals/UART/UART.h>
#include <PICo24/Peripherals/SPI/SPI.h>
#include <PICo24/Peripherals/I2C/I2C.h>
#include <PICo24/Peripherals/EXT_INT/EXT_INT.h>

// CONFIG4
#pragma config DSWDTPS = DSWDTPSF    //DSWDT Postscale Select->1:2,147,483,648 (25.7 days)
#pragma config DSWDTOSC = LPRC    //Deep Sleep Watchdog Timer Oscillator Select->DSWDT uses Low Power RC Oscillator (LPRC)
#pragma config RTCOSC = SOSC    //RTCC Reference Oscillator  Select->RTCC uses Secondary Oscillator (SOSC)
#pragma config DSBOREN = ON    //Deep Sleep BOR Enable bit->BOR enabled in Deep Sleep
#pragma config DSWDTEN = ON    //Deep Sleep Watchdog Timer->DSWDT enabled

// CONFIG3
#pragma config WPFP = WPFP63    //Write Protection Flash Page Segment Boundary->Highest Page (same as page 42)
#pragma config SOSCSEL = IO    //Secondary Oscillator Pin Mode Select->SOSC pins have digital I/O functions (RA4, RB4)
#pragma config WUTSEL = LEG    //Voltage Regulator Wake-up Time Select->Default regulator start-up time used
#pragma config WPDIS = WPDIS    //Segment Write Protection Disable->Segmented code protection disabled
#pragma config WPCFG = WPCFGDIS    //Write Protect Configuration Page Select->Last page and Flash Configuration words are unprotected
#pragma config WPEND = WPENDMEM    //Segment Write Protection End Page Select->Write Protect from WPFP to the last page of memory

// CONFIG2
#pragma config POSCMOD = NONE    //Primary Oscillator Select->Primary Oscillator disabled
#pragma config I2C1SEL = PRI    //I2C1 Pin Select bit->Use default SCL1/SDA1 pins for I2C1
#pragma config IOL1WAY = OFF    //IOLOCK One-Way Set Disable->Once set, the IOLOCK bit can be cleared
#pragma config OSCIOFNC = ON    //OSCO Pin Configuration->OSCO pin functions as port I/O (RA3)
#pragma config FCKSM = CSECMD    //Clock Switching and Fail-Safe Clock Monitor->Sw Enabled, Mon Disabled
#pragma config FNOSC = FRC    //Initial Oscillator Select->FRC
#pragma config PLL96MHZ = ON    //96MHz PLL Startup Select->96 MHz PLL Startup is enabled automatically on start-up
#pragma config PLLDIV = DIV2    //USB 96 MHz PLL Prescaler Select->Oscillator input divided by 2 (8 MHz input)
#pragma config IESO = OFF    //Internal External Switchover->IESO mode (Two-Speed Start-up) disabled

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
const char PICo24_Board_Name[] = "PotatoChip PICo24";
const char PICo24_Board_ChipManufacturer[] = "Microchip";
const char PICo24_Board_Chip[] = "PIC24FJ64GB002";

void PICo24_Clock_Initialize() {
	// CPDIV 1:1; PLLEN disabled; RCDIV FRC/1; DOZE 1:8; DOZEN disabled; ROI disabled;
	CLKDIV = 0x3000;
	// TUN Center frequency;
	OSCTUN = 0x00;
	// ROEN disabled; ROSEL FOSC; RODIV 0; ROSSLP disabled;
	REFOCON = 0x00;
	// ADC1MD enabled; T3MD enabled; T4MD enabled; T1MD enabled; U2MD enabled; T2MD enabled; U1MD enabled; SPI2MD enabled; SPI1MD enabled; T5MD enabled; I2C1MD enabled;
	PMD1 = 0x00;
	// OC5MD enabled; IC5MD enabled; IC4MD enabled; IC3MD enabled; OC1MD enabled; IC2MD enabled; OC2MD enabled; IC1MD enabled; OC3MD enabled; OC4MD enabled;
	PMD2 = 0x00;
	// PMPMD enabled; RTCCMD enabled; CMPMD enabled; CRCMD enabled; I2C2MD enabled;
	PMD3 = 0x00;
	// UPWMMD enabled; USB1MD enabled; CTMUMD enabled; REFOMD enabled; LVDMD enabled;
	PMD4 = 0x00;
	// CF no clock failure; NOSC FRCPLL; SOSCEN disabled; POSCEN disabled; CLKLOCK unlocked; OSWEN Switch is Complete; IOLOCK not-active;
	__builtin_write_OSCCONH((uint8_t) (0x01));
	__builtin_write_OSCCONL((uint8_t) (0x01));

	// Wait for Clock switch to occur
	while (OSCCONbits.OSWEN != 0);
	while (OSCCONbits.LOCK != 1);
}


