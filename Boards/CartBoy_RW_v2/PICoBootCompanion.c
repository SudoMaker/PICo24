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

#include "PICoBootCompanion.h"
#include "PICo24_Board.h"
#include <PICo24/PICoBootCompanion/PICoBootCompanion.h>

__attribute__((persistent, address(0x4600))) uint8_t __picoboot_runtime_env[sizeof(PICoBootRuntimeEnvironment)];

void PICoBoot_RuntimeEnvironment_Load() {
	uint8_t *penv = (uint8_t *) &picoboot_runtime_env;

	for (size_t i=0; i<sizeof(PICoBootRuntimeEnvironment); i++) {
		penv[i] = __picoboot_runtime_env[i];
	}
}

void PICoBoot_RuntimeEnvironment_Save() {
	uint8_t *penv = (uint8_t *) &picoboot_runtime_env;

	for (size_t i=0; i<sizeof(PICoBootRuntimeEnvironment); i++) {
		__picoboot_runtime_env[i] = penv[i];
	}
}