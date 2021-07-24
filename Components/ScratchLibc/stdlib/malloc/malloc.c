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

#include "malloc.h"

void __attribute__((__section__(".libc"))) *malloc(size_t size) {
	return umm_malloc(size);
}

void __attribute__((__section__(".libc"))) *calloc(size_t num, size_t size) {
	return umm_calloc(num, size);
}

void __attribute__((__section__(".libc"))) *realloc(void *ptr, size_t size) {
	return umm_realloc(ptr, size);
}

void __attribute__((__section__(".libc"))) free(void *ptr) {
	umm_free(ptr);
}

#ifdef __HAS_EDS__

auto_eds void *malloc_eds(uint32_t size) {
	return umm_eds_malloc(size);
}

auto_eds void *calloc_eds(uint32_t num, uint32_t size) {
	return umm_eds_calloc(num, size);
}

auto_eds void *realloc_eds(auto_eds void *ptr, uint32_t size) {
	return umm_eds_realloc(ptr, size);
}

void free_eds(auto_eds void *ptr) {
	umm_eds_free(ptr);
}

#endif

