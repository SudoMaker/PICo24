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

#include "SafeMalloc.h"
#include <ScratchLibc/ScratchLibc.h>

void *malloc_safe(size_t len) {
	void *ret = umm_malloc(len);

	printf("malloc: len=%u, ptr=%p, ptrend=%p\n", len, ret, ret + len);

	if (ret == NULL) {
		printf("malloc TRAP: NULL returned\n");
		while (1);
	}

	return ret;
}

void *calloc_safe(size_t n, size_t len) {
	void *ret = umm_calloc(n, len);

	printf("calloc: n=%u, len=%u, ptr=%p, ptrend=%p\n", n, len, ret, ret + len);

	if (ret == NULL) {
		printf("calloc TRAP: NULL returned\n");
		while (1);
	}

	return ret;
}

void *realloc_safe(void *p, size_t len) {
	void *ret = umm_realloc(p, len);

	printf("realloc: len=%u, oldptr=%p, newptr=%p, newptrend=%p\n", len, p, ret, ret + len);

	if (ret == NULL) {
		printf("realloc TRAP: NULL returned\n");
		while (1);
	}

	return ret;
}

void free_safe(void *p) {
	printf("free: ptr=%p\n", p);

	if (p == NULL) {
		printf("free TRAP: NULL encountered\n");
		while (1);
	}

	umm_free(p);
}