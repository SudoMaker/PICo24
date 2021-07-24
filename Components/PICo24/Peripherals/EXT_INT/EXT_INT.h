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

#pragma once

#include <stdint.h>

typedef struct {
	void (*callback)(void *userp);
	void *userp;
	volatile uint16_t *IEC;
	volatile uint16_t *IFS;
	volatile uint16_t *INTCON;
	uint8_t IEC_Offset;
	uint8_t IFS_Offset;
	uint8_t INTCON_Offset;
} EXTINT_HandleTypeDef;

enum {
	EXTINT_PositiveEdge = 0x0,
	EXTINT_NegativeEdge = 0x1,
};



extern void EXTINT_Initialize(EXTINT_HandleTypeDef *hextint, uint8_t extint_mode, void (*callback)(void *userp), void *userp);
extern void EXTINT_Enable(EXTINT_HandleTypeDef *hextint);
extern void EXTINT_Disable(EXTINT_HandleTypeDef *hextint);
extern void EXTINT_RunCallback(EXTINT_HandleTypeDef *hextint);