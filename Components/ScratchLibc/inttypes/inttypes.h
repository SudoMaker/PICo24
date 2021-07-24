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

#define PRIu8	"u"
#define PRIu16	"u"
#define PRIu32	"lu"
#define PRIu64	"llu"

#define PRId8	"d"
#define PRId16	"d"
#define PRId32	"ld"
#define PRId64	"lld"

#define PRIx8	"x"
#define PRIx16	"x"
#define PRIx32	"lx"
#define PRIx64	"llx"

#ifndef size_t
#define size_t		uint16_t
#endif

#define SIZE_MAX	UINT16_MAX

#ifndef ssize_t
#define ssize_t		int16_t
#endif

