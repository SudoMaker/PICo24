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

#include "Timer.h"

void Timer_Initialize(Timer_HandleTypeDef *htimer, uint16_t flags) {
	*((uint16_t *)htimer->CON) = 0x0;

	if (flags & TIMER_STOP_IN_IDLE) {
		htimer->CON->TSIDL = 1;
	}

	if (flags & TIMER_32BIT) {
		htimer->CON->T32 = 1;
	}

	if (flags & TIMER_GATED) {
		htimer->CON->TGATE = 1;
	}

	if (flags & TIMER_EXTCLK) {
		htimer->CON->TCS = 1;
	}

	if (flags & TIMER_SYNC) {
		htimer->CON->TSYNC = 1;
	}
}

void Timer_SetSpeedByPrescaler(Timer_HandleTypeDef *htimer, uint16_t prescaler) {
	htimer->CON->TCKPS = prescaler;
}

void Timer_Start(Timer_HandleTypeDef *htimer) {
	htimer->CON->TON = 1;
}

void Timer_Stop(Timer_HandleTypeDef *htimer) {
	htimer->CON->TON = 0;
}

void Timer_SetPeriod(Timer_HandleTypeDef *htimer, uint32_t period) {
	*htimer->PR = period & 0xffff;

	if (htimer->CON->T32) {
		*htimer->UPPER->PR = period >> 16;
	}
}

void Timer_SetValue(Timer_HandleTypeDef *htimer, uint32_t value) {
	htimer->CON->TON = 0;

	*htimer->VAL = value & 0xffff;

	if (htimer->CON->T32) {
		*htimer->UPPER->VAL = value >> 16;
	}

	htimer->CON->TON = 1;
}

uint32_t Timer_GetValue(Timer_HandleTypeDef *htimer) {
	uint32_t ret = *htimer->VAL;

	if (htimer->CON->T32) {
		ret |= ((uint32_t)*htimer->UPPER->VALHLD << 16);
	}

	return ret;
}

void Timer_WaitUntil(Timer_HandleTypeDef *htimer, uint32_t value) {
	while (value < Timer_GetValue(htimer));
}

void Timer_ClearInterrupt(Timer_HandleTypeDef *htimer) {
	*htimer->IFS &= ~(1U << htimer->IFS_OFFSET);
}

void Timer_SetInterrupt(Timer_HandleTypeDef *htimer, bool enabled) {
	Timer_ClearInterrupt(htimer);

	if (enabled) {
		*htimer->IEC |= 1U << htimer->IEC_OFFSET;
	} else {
		*htimer->IEC &= ~(1U << htimer->IEC_OFFSET);
	}
}

void Timer_SetInterruptHandler(Timer_HandleTypeDef *htimer, void (*handler)(void *), void *userp) {
	htimer->InterruptHandler = handler;
	htimer->UserP = userp;
}
