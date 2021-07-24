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

#include <PICo24_Board.h>

#include "mini_termios.h"
#include "mini_unistd.h"

int cfsetspeed(int fd, uint16_t speed) {
	switch (fd) {
#ifdef PINFUNC_UART1_TX
		case UART1_FILENO:
			U1BRG = speed;
			break;
#endif
#ifdef PINFUNC_UART2_TX
		case UART2_FILENO:
			U2BRG = speed;
			break;
#endif
#ifdef PINFUNC_UART3_TX
		case UART3_FILENO:
			U3BRG = speed;
			break;
#endif
		default:
			return -1;
	}

	return 0;
}

int cfgetspeed(int fd, uint16_t speed) {
	switch (fd) {
#ifdef PINFUNC_UART1_TX
		case UART1_FILENO:
			return U1BRG;
#endif
#ifdef PINFUNC_UART2_TX
		case UART2_FILENO:
			return U2BRG;
#endif
#ifdef PINFUNC_UART3_TX
		case UART3_FILENO:
			return U3BRG;
#endif
		default:
			return -1;
	}
}