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

#include "umm_malloc/umm_malloc_cfg.h"
#include "umm_malloc/umm_malloc.h"

#ifdef __HAS_EDS__

#include "umm_malloc_eds/umm_eds_malloc.h"

extern auto_eds void *malloc_eds(uint32_t size);
extern auto_eds void *calloc_eds(uint32_t num, uint32_t size);
extern auto_eds void *realloc_eds(auto_eds void *ptr, uint32_t size);
extern void free_eds(auto_eds void *ptr);

#endif

extern void *malloc(size_t size);
extern void *calloc(size_t num, size_t size);
extern void *realloc(void *ptr, size_t size);
extern void free(void *ptr);

