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

#include <stdlib.h>
#include <stdint.h>
#include <PICo24/Core/Core.h>


__extension__ typedef struct {
	union {
		struct {
			uint16_t SPIRBF:1;
			uint16_t SPITBF:1;
			uint16_t SISEL:3;
			uint16_t SRXMPT:1;
			uint16_t SPIROV:1;
			uint16_t SRMPT:1;
			uint16_t SPIBEC:3;
			uint16_t :2;
			uint16_t SPISIDL:1;
			uint16_t :1;
			uint16_t SPIEN:1;
		};
		struct {
			uint16_t :2;
			uint16_t SISEL0:1;
			uint16_t SISEL1:1;
			uint16_t SISEL2:1;
			uint16_t :3;
			uint16_t SPIBEC0:1;
			uint16_t SPIBEC1:1;
			uint16_t SPIBEC2:1;
		};
	};
} SPISTATBITS;

__extension__ typedef struct {
	union {
		struct {
			uint16_t PPRE:2;
			uint16_t SPRE:3;
			uint16_t MSTEN:1;
			uint16_t CKP:1;
			uint16_t SSEN:1;
			uint16_t CKE:1;
			uint16_t SMP:1;
			uint16_t MODE16:1;
			uint16_t DISSDO:1;
			uint16_t DISSCK:1;
		};
		struct {
			uint16_t PPRE0:1;
			uint16_t PPRE1:1;
			uint16_t SPRE0:1;
			uint16_t SPRE1:1;
			uint16_t SPRE2:1;
		};
	};
} SPICON1BITS;

typedef struct {
	uint16_t SPIBEN:1;
	uint16_t SPIFE:1;
	uint16_t :11;
	uint16_t SPIFPOL:1;
	uint16_t SPIFSD:1;
	uint16_t FRMEN:1;
} SPICON2BITS;

typedef struct {
	volatile SPISTATBITS *STAT;
	volatile SPICON1BITS *CON1;
	volatile SPICON1BITS *CON2;
	volatile uint16_t *BUF;
} SPI_HandleTypeDef;

enum {
	SPI_MASTER = 0x1,
	SPI_SLAVE = 0x0,

	SPI_CPHA = 0x2,
	SPI_CPOL = 0x4,

	SPI_MODE_0 = (0|0),
	SPI_MODE_1 = (0|SPI_CPHA),
	SPI_MODE_2 = (SPI_CPOL|0),
	SPI_MODE_3 = (SPI_CPOL|SPI_CPHA),

	SPI_16BIT = 0x10,

	SPI_NO_CK = 0x20,
	SPI_NO_CS = 0x40,
	SPI_NO_SO = 0x80,
};

enum {
	SPI_PPRE_64_1 = 0,
	SPI_PPRE_16_1,
	SPI_PPRE_4_1,
	SPI_PPRE_1_1
};

enum {
	SPI_SPRE_8_1 = 0,
	SPI_SPRE_7_1,
	SPI_SPRE_6_1,
	SPI_SPRE_5_1,
	SPI_SPRE_4_1,
	SPI_SPRE_3_1,
	SPI_SPRE_2_1,
	SPI_SPRE_1_1,
};

extern const SPI_HandleTypeDef hspi1;
extern const SPI_HandleTypeDef hspi2;
extern const SPI_HandleTypeDef hspi3;

extern void SPI_Initialize(const SPI_HandleTypeDef *hspi, uint16_t spi_mode);
extern void SPI_SetSpeedByPrescaler(const SPI_HandleTypeDef *hspi, uint8_t ppre, uint8_t spre);

extern void SPI_Enable(const SPI_HandleTypeDef *hspi);
extern void SPI_Disable(const SPI_HandleTypeDef *hspi);

extern uint16_t SPI_TransmitReceive(const SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
extern uint16_t SPI_Transmit(const SPI_HandleTypeDef *hspi, const uint8_t *pTxData, uint16_t Size);
extern uint16_t SPI_Receive(const SPI_HandleTypeDef *hspi, uint8_t *pRxData, uint16_t Size);

#ifdef __HAS_EDS__
extern uint16_t SPI_TransmitReceive_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pTxData, auto_eds uint8_t *pRxData, uint16_t Size);
extern uint16_t SPI_Transmit_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pTxData, uint16_t Size);
extern uint16_t SPI_Receive_EDS(const SPI_HandleTypeDef *hspi, auto_eds uint8_t *pRxData, uint16_t Size);
#endif