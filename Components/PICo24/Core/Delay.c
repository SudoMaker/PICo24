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

#include "Delay.h"

#include <stdint.h>

void Delay_Milliseconds(uint64_t milliseconds) {
    while (milliseconds--) {
	    __delay32(FCY_DIV_1000);
    }
}

void Delay_Microseconds(uint64_t microseconds) {
    while (microseconds >= 32) {
	    __delay32(FCY_DIV_1000000 * 32);

	    microseconds -= 32;
    }
    
    while (microseconds--) {
	    __delay32(FCY_DIV_1000000);
    }
}
