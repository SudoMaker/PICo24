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

#include <PICo24/Core/IDESupport.h>

#include "../inttypes/inttypes.h"

#ifdef __HAS_EDS__
extern auto_eds void *memset_eds(auto_eds void *p, int c, uint32_t len);
extern auto_eds void *memcpy_eds(auto_eds void *dest, auto_eds const void *src, uint32_t len);
extern auto_eds void *memmove_eds(auto_eds void *dest, auto_eds const void *src, uint32_t len);
extern int memcmp_eds(auto_eds const void *s1, auto_eds const void *s2, uint32_t n);
#endif