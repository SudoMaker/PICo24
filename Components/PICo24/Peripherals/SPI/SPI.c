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


#include "SPI.h"

#ifdef PICo24_Enable_Peripheral_SPI
void SPI_Initialize(const SPI_HandleTypeDef *hspi, uint16_t spi_mode) {
	volatile SPICON1BITS *con1 = hspi->CON1;

	con1->SMP = 0;

	if (spi_mode & SPI_MASTER)
		con1->MSTEN = 1;
	else
		con1->MSTEN = 0;

	if (spi_mode & SPI_NO_SO)
		con1->DISSDO = 1;
	else
		con1->DISSDO = 0;

	if (spi_mode & SPI_NO_CK)
		con1->DISSCK = 1;
	else
		con1->DISSCK = 0;

	if (spi_mode & SPI_NO_CS)
		con1->SSEN = 0;
	else
		con1->SSEN = 1;

	if (spi_mode & SPI_16BIT)
		con1->MODE16 = 1;
	else
		con1->MODE16 = 0;

	if (spi_mode & SPI_CPHA)
		con1->CKE = 1;
	else
		con1->CKE = 0;

	if (spi_mode & SPI_CPOL)
		con1->CKP = 1;
	else
		con1->CKP = 0;

	*((uint16_t *)hspi->CON2) = 0x01;
	*((uint16_t *)hspi->STAT) = 0x800c;
}

void SPI_SetSpeedByPrescaler(const SPI_HandleTypeDef *hspi, uint8_t ppre, uint8_t spre) {
	volatile SPICON1BITS *con1 = hspi->CON1;

	con1->PPRE = ppre;
	con1->SPRE = spre;
}

void SPI_Enable(const SPI_HandleTypeDef *hspi) {
	hspi->STAT->SPIEN = 1;
}

void SPI_Disable(const SPI_HandleTypeDef *hspi) {
	hspi->STAT->SPIEN = 0;
}

uint16_t SPI_TransmitReceive(const SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size) {
	uint16_t dataSentCount = 0;
	uint16_t dataReceivedCount = 0;
	uint16_t dummyDataReceived = 0;
	uint16_t dummyDataTransmit = 0;

	uint8_t *pSend, *pReceived;
	uint16_t addressIncrement;
	uint16_t receiveAddressIncrement, sendAddressIncrement;

	addressIncrement = 1;

	if (pTxData == NULL) {
		sendAddressIncrement = 0;
		pSend = (uint8_t*)&dummyDataTransmit;
	} else {
		sendAddressIncrement = addressIncrement;
		pSend = (uint8_t*)pTxData;
	}

	if (pRxData == NULL) {
		receiveAddressIncrement = 0;
		pReceived = (uint8_t*)&dummyDataReceived;
	} else {
		receiveAddressIncrement = addressIncrement;
		pReceived = (uint8_t*)pRxData;
	}

	while (dataSentCount < Size) {
		// Workaround for PIC24FJ64GB004 SPITBF bugs
		if (hspi->STAT->SPIBEC != 7) {
			*hspi->BUF = *pSend;

			pSend += sendAddressIncrement;
			dataSentCount++;
		}

		// Reimu 20210525:
		// Sometimes the RX FIFO will overflow (looks like a timing issue, TX 1 byte and RX 0 byte, SPI mode?), and the last while
		// loop in this function will stuck forever because the missing bytes will make `dataReceivedCount' never be able to equal `Size'.
		// So we changed the `if' to `while' to drain the RX FIFO as much as we can.
		// We're glad that we figured this out without asking MCHP's “Warm” support team for help.
		// Someone noticed this behavior as well, but it remained a mystery for them.
		// https://electronics.stackexchange.com/questions/296086/pic24f-mplab-x-mcc-microchip-code-configurator-spi-driver-issue
		// Don't make your code quality the same as Windows 10, dear MCHP.
		while (hspi->STAT->SRXMPT == 0) {
			*pReceived = *hspi->BUF;

			pReceived += receiveAddressIncrement;
			dataReceivedCount++;
		}

	}

	while (dataReceivedCount < Size) {
		if (hspi->STAT->SRXMPT == 0) {

			*pReceived = *hspi->BUF;

			pReceived += receiveAddressIncrement;
			dataReceivedCount++;
		}
	}

	return dataSentCount;
}


uint16_t SPI_Transmit(const SPI_HandleTypeDef *hspi, const uint8_t *pTxData, uint16_t Size) {
	// A trimmed version to make TX only operations faster.

	uint16_t dataSentCount = 0;
	uint16_t dataReceivedCount = 0;

	while (dataSentCount < Size) {
		if (hspi->STAT->SPIBEC != 7) {
			*hspi->BUF = *pTxData;
			pTxData += 1;
			dataSentCount++;
		}

		while (hspi->STAT->SRXMPT == 0) {
			PICo24_Discard16 = *hspi->BUF;
			dataReceivedCount++;
		}
	}

	while (dataReceivedCount < Size) {
		if (hspi->STAT->SRXMPT == 0) {
			PICo24_Discard16 = *hspi->BUF;
			dataReceivedCount++;
		}
	}

	return dataSentCount;
}

uint16_t SPI_Receive(const SPI_HandleTypeDef *hspi, uint8_t *pRxData, uint16_t Size) {
	return SPI_TransmitReceive(hspi, NULL, pRxData, Size);
}

#ifdef __HAS_EDS__
uint16_t SPI_TransmitReceive_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pTxData, auto_eds uint8_t *pRxData, uint16_t Size) {
	uint16_t dataSentCount = 0;
	uint16_t dataReceivedCount = 0;
	uint16_t dummyDataReceived = 0;
	uint16_t dummyDataTransmit = 0;

	auto_eds uint8_t *pSend, *pReceived;
	uint16_t addressIncrement;
	uint16_t receiveAddressIncrement, sendAddressIncrement;

	addressIncrement = 1;

	if (pTxData == NULL) {
		sendAddressIncrement = 0;
		pSend = (auto_eds uint8_t*)&dummyDataTransmit;
	} else {
		sendAddressIncrement = addressIncrement;
		pSend = (auto_eds uint8_t*)pTxData;
	}

	if (pRxData == NULL) {
		receiveAddressIncrement = 0;
		pReceived = (auto_eds uint8_t*)&dummyDataReceived;
	} else {
		receiveAddressIncrement = addressIncrement;
		pReceived = (auto_eds uint8_t*)pRxData;
	}

	while (dataSentCount < Size) {
		// Workaround for PIC24FJ64GB004 SPITBF bugs
		if (hspi->STAT->SPIBEC != 7) {
			*hspi->BUF = *pSend;

			pSend += sendAddressIncrement;
			dataSentCount++;
		}

		// Reimu 20210525:
		// Sometimes the RX FIFO will overflow (looks like a timing issue, TX 1 byte and RX 0 byte, SPI mode?), and the last while
		// loop in this function will stuck forever because the missing bytes will make `dataReceivedCount' never be able to equal `Size'.
		// So we changed the `if' to `while' to drain the RX FIFO as much as we can.
		// We're glad that we figured this out without asking MCHP's “Warm” support team for help.
		// Someone noticed this behavior as well, but it remained a mystery for them.
		// https://electronics.stackexchange.com/questions/296086/pic24f-mplab-x-mcc-microchip-code-configurator-spi-driver-issue
		// Don't make your code quality the same as Windows 10, dear MCHP.
		while (hspi->STAT->SRXMPT == 0) {
			*pReceived = *hspi->BUF;

			pReceived += receiveAddressIncrement;
			dataReceivedCount++;
		}

	}

	while (dataReceivedCount < Size) {
		if (hspi->STAT->SRXMPT == 0) {

			*pReceived = *hspi->BUF;

			pReceived += receiveAddressIncrement;
			dataReceivedCount++;
		}
	}

	return dataSentCount;
}


uint16_t SPI_Transmit_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pTxData, uint16_t Size) {
	// A trimmed version to make TX only operations faster.

	uint16_t dataSentCount = 0;
	uint16_t dataReceivedCount = 0;

	while (dataSentCount < Size) {
		if (hspi->STAT->SPIBEC != 7) {
			*hspi->BUF = *pTxData;
			pTxData += 1;
			dataSentCount++;
		}

		while (hspi->STAT->SRXMPT == 0) {
			PICo24_Discard16 = *hspi->BUF;
			dataReceivedCount++;
		}
	}

	while (dataReceivedCount < Size) {
		if (hspi->STAT->SRXMPT == 0) {
			PICo24_Discard16 = *hspi->BUF;
			dataReceivedCount++;
		}
	}

	return dataSentCount;
}

uint16_t SPI_Receive_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pRxData, uint16_t Size) {
	return SPI_TransmitReceive_EDS(hspi, NULL, pRxData, Size);
}

#endif

#endif