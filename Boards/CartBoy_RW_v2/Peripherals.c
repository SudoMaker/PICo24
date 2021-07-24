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
#include <PICo24/Peripherals/Timer/Timer.h>
#include <PICo24/Peripherals/USB/usb_deluxe.h>

#ifdef PICo24_Enable_Peripheral_TIMER

Timer_HandleTypeDef htimer1 = {
	.CON = (volatile TMRCON *) &T1CON,
	.PR = &PR1,
	.VAL = &TMR1,
	.VALHLD = NULL,
	.UPPER = NULL,
	.IFS = &IFS0,
	.IFS_OFFSET = 3,
	.IEC = &IEC0,
	.IEC_OFFSET = 3,
};

Timer_HandleTypeDef htimer3 = {
	.CON = (volatile TMRCON *) &T3CON,
	.PR = &PR3,
	.VAL = &TMR3,
	.VALHLD = &TMR3HLD,
	.UPPER = NULL,
	.IFS = &IFS0,
	.IFS_OFFSET = 8,
	.IEC = &IEC0,
	.IEC_OFFSET = 8,
};

Timer_HandleTypeDef htimer2 = {
	.CON = (volatile TMRCON *) &T2CON,
	.PR = &PR2,
	.VAL = &TMR2,
	.VALHLD = NULL,
	.UPPER = &htimer3,
	.IFS = &IFS0,
	.IFS_OFFSET = 7,
	.IEC = &IEC0,
	.IEC_OFFSET = 7,
};

Timer_HandleTypeDef htimer5 = {
	.CON = (volatile TMRCON *) &T5CON,
	.PR = &PR5,
	.VAL = &TMR5,
	.VALHLD = &TMR5HLD,
	.UPPER = NULL,
	.IFS = &IFS1,
	.IFS_OFFSET = 12,
	.IEC = &IEC1,
	.IEC_OFFSET = 12,
};

Timer_HandleTypeDef htimer4 = {
	.CON = (volatile TMRCON *) &T4CON,
	.PR = &PR4,
	.VAL = &TMR4,
	.VALHLD = NULL,
	.UPPER = &htimer5,
	.IFS = &IFS1,
	.IFS_OFFSET = 11,
	.IEC = &IEC1,
	.IEC_OFFSET = 11,
};

#endif

#ifdef PICo24_Enable_Peripheral_UART
const UART_HandleTypeDef huart1 = {
	.STA = (volatile UARTSTABITS *) &U1STA,
	.MODE = (volatile UARTMODEBITS *) &U1MODE,
	.BRG = &U1BRG,
	.TXREG = &U1TXREG,
	.RXREG = &U1RXREG
};

const UART_HandleTypeDef huart2 = {
	.STA = (volatile UARTSTABITS *) &U2STA,
	.MODE = (volatile UARTMODEBITS *) &U2MODE,
	.BRG = &U2BRG,
	.TXREG = &U2TXREG,
	.RXREG = &U2RXREG
};

const UART_HandleTypeDef huart3 = {
	.STA = (volatile UARTSTABITS *) &U3STA,
	.MODE = (volatile UARTMODEBITS *) &U3MODE,
	.BRG = &U3BRG,
	.TXREG = &U3TXREG,
	.RXREG = &U3RXREG
};
#endif

#ifdef PICo24_Enable_Peripheral_SPI
const SPI_HandleTypeDef hspi1 = {
	.STAT = (SPISTATBITS *) &SPI1STAT,
	.CON1 = (SPICON1BITS *) &SPI1CON1,
	.CON2 = (SPICON1BITS *) &SPI1CON2,
	.BUF = &SPI1BUF
};

const SPI_HandleTypeDef hspi2 = {
	.STAT = (SPISTATBITS *) &SPI2STAT,
	.CON1 = (SPICON1BITS *) &SPI2CON1,
	.CON2 = (SPICON1BITS *) &SPI2CON2,
	.BUF = &SPI2BUF
};


const SPI_HandleTypeDef hspi3 = {
	.STAT = (SPISTATBITS *) &SPI3STAT,
	.CON1 = (SPICON1BITS *) &SPI3CON1,
	.CON2 = (SPICON1BITS *) &SPI3CON2,
	.BUF = &SPI3BUF
};
#endif

#ifdef PICo24_Enable_Peripheral_EXTINT
EXTINT_HandleTypeDef hextint0 = {
	.IEC = &IEC0,
	.IFS = &IFS0,
	.INTCON = &INTCON2,
	.IEC_Offset = 0,
	.IFS_Offset = 0,
	.INTCON_Offset = 0
};

EXTINT_HandleTypeDef hextint1 = {
	.IEC = &IEC1,
	.IFS = &IFS1,
	.INTCON = &INTCON2,
	.IEC_Offset = 4,
	.IFS_Offset = 4,
	.INTCON_Offset = 1
};

EXTINT_HandleTypeDef hextint2 = {
	.IEC = &IEC1,
	.IFS = &IFS1,
	.INTCON = &INTCON2,
	.IEC_Offset = 13,
	.IFS_Offset = 13,
	.INTCON_Offset = 2
};

EXTINT_HandleTypeDef hextint3 = {
	.IEC = &IEC3,
	.IFS = &IFS3,
	.INTCON = &INTCON2,
	.IEC_Offset = 5,
	.IFS_Offset = 5,
	.INTCON_Offset = 3
};

EXTINT_HandleTypeDef hextint4 = {
	.IEC = &IEC3,
	.IFS = &IFS3,
	.INTCON = &INTCON2,
	.IEC_Offset = 6,
	.IFS_Offset = 6,
	.INTCON_Offset = 4
};

void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt() {
	EXTINT_RunCallback(&hextint0);
}

void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt() {
	EXTINT_RunCallback(&hextint1);
}

void __attribute__ ((interrupt, no_auto_psv)) _INT2Interrupt() {
	EXTINT_RunCallback(&hextint2);
}

void __attribute__ ((interrupt, no_auto_psv)) _INT3Interrupt() {
	EXTINT_RunCallback(&hextint3);
}

void __attribute__ ((interrupt, no_auto_psv)) _INT4Interrupt() {
	EXTINT_RunCallback(&hextint4);
}
#endif

#ifdef PICo24_Enable_Peripheral_I2C
I2C_HandleTypeDef hi2c1 = {
	.STAT = (volatile I2CSTATBITS *) &I2C1STAT,
	.CON = (volatile I2CCONBITS *) &I2C1CON,
	.BRG = &I2C1BRG,
	.TRN = &I2C1TRN,
	.RCV = &I2C1RCV,
	.IEC = &IEC1,
	.IFS = &IFS1,
	.IECOffset_M = 1,
	.IECOffset_S = 0,
	.IFSOffset_M = 1,
	.IFSOffset_S = 0,
};

I2C_HandleTypeDef hi2c2 = {
	.STAT = (volatile I2CSTATBITS *) &I2C2STAT,
	.CON = (volatile I2CCONBITS *) &I2C2CON,
	.BRG = &I2C2BRG,
	.TRN = &I2C2TRN,
	.RCV = &I2C2RCV,
	.IEC = &IEC3,
	.IFS = &IFS3,
	.IECOffset_M = 2,
	.IECOffset_S = 1,
	.IFSOffset_M = 2,
	.IFSOffset_S = 1,
};

I2C_HandleTypeDef hi2c3 = {
	.STAT = (volatile I2CSTATBITS *) &I2C3STAT,
	.CON = (volatile I2CCONBITS *) &I2C3CON,
	.BRG = &I2C3BRG,
	.TRN = &I2C3TRN,
	.RCV = &I2C3RCV,
	.IEC = &IEC5,
	.IFS = &IFS5,
	.IECOffset_M = 5,
	.IECOffset_S = 4,
	.IFSOffset_M = 5,
	.IFSOffset_S = 4,
};

void __attribute__ ((interrupt, no_auto_psv)) _MI2C1Interrupt() {
	I2C_Master_ProcessInterrupt(&hi2c1);
}

void __attribute__ ((interrupt, no_auto_psv)) _MI2C2Interrupt() {
	I2C_Master_ProcessInterrupt(&hi2c2);
}

void __attribute__ ((interrupt, no_auto_psv)) _MI2C3Interrupt() {
	I2C_Master_ProcessInterrupt(&hi2c3);
}
#endif



void __attribute__((interrupt,auto_psv)) _USB1Interrupt() {
	if (usb_deluxe_role == USB_ROLE_DEVICE) {
#ifdef PICo24_Enable_Peripheral_USB_DEVICE
		USBDeviceTasks();
#endif
	} else if (usb_deluxe_role == USB_ROLE_HOST) {
#ifdef PICo24_Enable_Peripheral_USB_HOST
		USB_HostInterruptHandler();
#endif
	} else {
		USBClearUSBInterrupt();
		USBDisableInterrupts();
	}
}