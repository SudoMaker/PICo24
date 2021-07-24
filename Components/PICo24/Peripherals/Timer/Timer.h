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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef union {
	struct {
		uint16_t :1;
		uint16_t TCS:1;
		uint16_t TSYNC:1;
		uint16_t T32:1;
		uint16_t TCKPS:2;
		uint16_t TGATE:1;
		uint16_t :6;
		uint16_t TSIDL:1;
		uint16_t :1;
		uint16_t TON:1;
	};
	struct {
		uint16_t :4;
		uint16_t TCKPS0:1;
		uint16_t TCKPS1:1;
	};
} TMRCON;

enum {
	TIMER_16BIT = 0x0,
	TIMER_32BIT = 0x1,
	TIMER_GATED = 0x2,
	TIMER_EXTCLK = 0x4,
	TIMER_SYNC = 0x8,
	TIMER_STOP_IN_IDLE = 0x10,


	TIMER_PS_1_1 = 0x0,
	TIMER_PS_1_8 = 0x1,
	TIMER_PS_1_64 = 0x2,
	TIMER_PS_1_256 = 0x3,
};

typedef struct __Timer_HandleTypeDef {
	volatile TMRCON *CON;
	volatile uint16_t *VAL;
	volatile uint16_t *VALHLD;
	volatile uint16_t *PR;
	volatile uint16_t *IFS;
	volatile uint16_t *IEC;
	struct __Timer_HandleTypeDef *UPPER;
	void (*InterruptHandler)(void *);
	void *UserP;
	uint8_t IFS_OFFSET, IEC_OFFSET;
} Timer_HandleTypeDef;

extern void Timer_Initialize(Timer_HandleTypeDef *htimer, uint16_t flags);
extern void Timer_SetSpeedByPrescaler(Timer_HandleTypeDef *htimer, uint16_t prescaler);
extern void Timer_Start(Timer_HandleTypeDef *htimer);
extern void Timer_Stop(Timer_HandleTypeDef *htimer);
extern void Timer_SetPeriod(Timer_HandleTypeDef *htimer, uint32_t period);
extern void Timer_SetValue(Timer_HandleTypeDef *htimer, uint32_t value);
extern uint32_t Timer_GetValue(Timer_HandleTypeDef *htimer);
extern void Timer_ClearInterrupt(Timer_HandleTypeDef *htimer);
extern void Timer_SetInterrupt(Timer_HandleTypeDef *htimer, bool enabled);
extern void Timer_SetInterruptHandler(Timer_HandleTypeDef *htimer, void (*handler)(void *), void *userp);

extern Timer_HandleTypeDef htimer1;
extern Timer_HandleTypeDef htimer2;
extern Timer_HandleTypeDef htimer3;
extern Timer_HandleTypeDef htimer4;
extern Timer_HandleTypeDef htimer5;