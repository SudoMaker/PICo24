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

#include "EXT_INT.h"

#ifdef PICo24_Enable_Peripheral_EXTINT
void EXTINT_Initialize(EXTINT_HandleTypeDef *hextint, uint8_t extint_mode, void (*callback)(void *userp), void *userp) {
	hextint->callback = callback;
	hextint->userp = userp;

	if (extint_mode & EXTINT_NegativeEdge)
		*hextint->INTCON |= 1U << hextint->INTCON_Offset;
	else
		*hextint->INTCON &= ~(1U << hextint->INTCON_Offset);


	*hextint->IFS &= ~(1U << hextint->IFS_Offset);
}

void EXTINT_Enable(EXTINT_HandleTypeDef *hextint) {
	*hextint->IEC |= 1U << hextint->IEC_Offset;
}

void EXTINT_Disable(EXTINT_HandleTypeDef *hextint) {
	*hextint->IEC &= ~(1U << hextint->IEC_Offset);
}

void EXTINT_RunCallback(EXTINT_HandleTypeDef *hextint) {
	if (hextint->callback) {
		hextint->callback(hextint->userp);
	}
}
#endif