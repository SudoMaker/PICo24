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
#include <stdlib.h>
#include <string.h>

extern uint8_t Variant_AsUint8(void *data);
extern int8_t Variant_AsInt8(void *data);
extern uint16_t Variant_AsUint16(void *data);
extern int16_t Variant_AsInt16(void *data);
extern uint32_t Variant_AsUint32(void *data);
extern int32_t Variant_AsInt32(void *data);
extern uint64_t Variant_AsUint64(void *data);
extern int64_t Variant_AsInt64(void *data);
