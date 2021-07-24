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

#include "Vector_EDS.h"
#include <PICo24/Library/SafeMalloc.h>

#ifdef __HAS_EDS__

void Vector_EDS_Init(Vector_EDS *vec, size_t element_size) {
	memset(vec, 0, sizeof(Vector_EDS));
	vec->element_size = element_size;
}

void Vector_EDS_Clear(Vector_EDS *vec) {
	if (vec->size) {
		free_eds(vec->data);
		vec->size = 0;
		vec->data = NULL;
	}
}

void Vector_EDS_Resize(Vector_EDS *vec, size_t n) {
	if (vec->data) {
		vec->data = realloc_eds(vec->data, vec->element_size * n);
	} else {
		vec->data = malloc_eds(vec->element_size * n);
	}

	vec->size = n;
}

auto_eds void *Vector_EDS_At(Vector_EDS *vec, size_t pos) {
	if (pos >= vec->size) {
		return NULL;
	} else {
		return ((auto_eds uint8_t *) vec->data) + vec->element_size * pos;
	}
}

auto_eds void *Vector_EDS_At2(Vector_EDS *vec, size_t pos) {
	if (vec->size <= pos) {
		Vector_EDS_Resize(vec, pos+1);
		auto_eds void *ret = ((auto_eds uint8_t *)vec->data) + vec->element_size * pos;
		memset_eds(ret, 0, vec->element_size);
		return ret;
	}

	auto_eds void *ret = ((auto_eds uint8_t *)vec->data) + vec->element_size * pos;
	return ret;
}

long Vector_EDS_DistanceFromBegin(Vector_EDS *vec, auto_eds const void *it) {
	if (vec->element_size == 0) {
		return Vector_EDS_Npos;
	}

	return (it - vec->data) / vec->element_size;
}

auto_eds void *Vector_EDS_Front(Vector_EDS *vec) {
	return vec->data;
}

auto_eds void *Vector_EDS_Back(Vector_EDS *vec) {
	if (vec->size == 0) {
		return NULL;
	} else {
		return ((auto_eds uint8_t *) vec->data) + vec->element_size * (vec->size - 1);
	}
}

bool Vector_EDS_Empty(Vector_EDS *vec) {
	return vec->size == 0;
}

auto_eds void *Vector_EDS_EmplaceBack(Vector_EDS *vec) {
	Vector_EDS_Resize(vec, vec->size+1);

	return Vector_EDS_Back(vec);
}

auto_eds void *Vector_EDS_EmplaceBack2(Vector_EDS *vec, size_t len) {
	size_t oldsize = vec->size;
	Vector_EDS_Resize(vec, oldsize+len);

	return Vector_EDS_At(vec, oldsize);
}

void Vector_EDS_PushBack(Vector_EDS *vec, auto_eds const void *object) {
	memcpy_eds(Vector_EDS_EmplaceBack(vec), object, vec->element_size);
}

void Vector_EDS_PushBack2(Vector_EDS *vec, auto_eds const void *object, size_t len) {
	memcpy_eds(Vector_EDS_EmplaceBack2(vec, len), object, vec->element_size * len);
}

void Vector_EDS_Erase(Vector_EDS *vec, auto_eds const void *object) {
	uint16_t pos = Vector_EDS_DistanceFromBegin(vec, object);
	uint16_t items_left = vec->size - pos - 1;

	if (items_left) {
		memmove_eds((auto_eds void *)object, object + vec->element_size, items_left * vec->element_size);
	}

	vec->size--;
}

long Vector_EDS_Find(Vector_EDS *vec, auto_eds const void *object) {
	for (size_t i=0; i<vec->size; i++) {
		if (memcmp_eds(Vector_EDS_At(vec, i), object, vec->element_size) == 0) {
			return i;
		}
	}

	return Vector_EDS_Npos;
}

#endif
