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

#include "Vector.h"
#include <PICo24/Library/SafeMalloc.h>

void Vector_Init(Vector *vec, size_t element_size) {
	memset(vec, 0, sizeof(Vector));
	vec->element_size = element_size;
}

void Vector_Clear(Vector *vec) {
	if (vec->size) {
		free(vec->data);
		vec->size = 0;
		vec->data = NULL;
	}
}

void Vector_Resize(Vector *vec, size_t n) {
	if (vec->data) {
		vec->data = realloc(vec->data, vec->element_size * n);
	} else {
		vec->data = malloc(vec->element_size * n);
	}

	vec->size = n;
}

void *Vector_At(Vector *vec, size_t pos) {
	if (pos >= vec->size) {
		return NULL;
	} else {
		return ((uint8_t *) vec->data) + vec->element_size * pos;
	}
}

void *Vector_At2(Vector *vec, size_t pos) {
	if (vec->size <= pos) {
		Vector_Resize(vec, pos+1);
		void *ret = ((uint8_t *)vec->data) + vec->element_size * pos;
		memset(ret, 0, vec->element_size);
		return ret;
	}

	void *ret = ((uint8_t *)vec->data) + vec->element_size * pos;
	return ret;
}

long Vector_DistanceFromBegin(Vector *vec, const void *it) {
	if (vec->element_size == 0) {
		return Vector_Npos;
	}

	return (it - vec->data) / vec->element_size;
}

void *Vector_Front(Vector *vec) {
	return vec->data;
}

void *Vector_Back(Vector *vec) {
	if (vec->size == 0) {
		return NULL;
	} else {
		return ((uint8_t *) vec->data) + vec->element_size * (vec->size - 1);
	}
}

bool Vector_Empty(Vector *vec) {
	return vec->size == 0;
}

void *Vector_EmplaceBack(Vector *vec) {
	Vector_Resize(vec, vec->size+1);

	return Vector_Back(vec);
}

void *Vector_EmplaceBack2(Vector *vec, size_t len) {
	size_t oldsize = vec->size;
	Vector_Resize(vec, oldsize+len);

	return Vector_At(vec, oldsize);
}

void Vector_PushBack(Vector *vec, const void *object) {
	memcpy(Vector_EmplaceBack(vec), object, vec->element_size);
}

void Vector_PushBack2(Vector *vec, const void *object, size_t len) {
	memcpy(Vector_EmplaceBack2(vec, len), object, vec->element_size * len);
}

void Vector_Erase(Vector *vec, const void *object) {
	uint16_t pos = Vector_DistanceFromBegin(vec, object);
	uint16_t items_left = vec->size - pos - 1;

	if (items_left) {
		memmove((void *)object, object + vec->element_size, items_left * vec->element_size);
	}

	vec->size--;
}

long Vector_Find(Vector *vec, const void *object) {
	for (size_t i=0; i<vec->size; i++) {
		if (memcmp(Vector_At(vec, i), object, vec->element_size) == 0) {
			return i;
		}
	}

	return Vector_Npos;
}
