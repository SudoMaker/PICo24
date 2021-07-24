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

#include <PICo24/Core/Delay.h>
#include <ScratchLibc/ScratchLibc.h>

extern int STDIN_FILENO;
extern int STDOUT_FILENO;
extern int STDERR_FILENO;

enum {
	STDIN_FILENO_STD = 0,
	STDOUT_FILENO_STD = 1,
	STDERR_FILENO_STD = 2,

	UART_FILENO_MASK = 0x10,
	UART1_FILENO = 0x10,
	UART2_FILENO,
	UART3_FILENO,

	CDC_ACM_FILENO_MASK = 0x20,
	CDC_ACM0_FILENO = 0x20,
	CDC_ACM1_FILENO,
	CDC_ACM2_FILENO,
	CDC_ACM3_FILENO,

	CDC_NCM_FILENO_MASK = 0x40,
	CDC_NCM0_FILENO = 0x40,
	CDC_NCM1_FILENO,
	CDC_NCM2_FILENO,
	CDC_NCM3_FILENO,

	CDC_ECM_FILENO_MASK = 0x80,
	CDC_ECM0_FILENO = 0x80,
	CDC_ECM1_FILENO,
	CDC_ECM2_FILENO,
	CDC_ECM3_FILENO,
};

extern ssize_t write(int fd, const void *buf, size_t count);
extern ssize_t read(int fd, void *buf, size_t count);

extern void usleep(uint64_t usec);
extern void msleep(uint64_t msec);
extern void sleep(uint32_t sec);
