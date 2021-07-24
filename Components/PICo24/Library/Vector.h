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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	size_t size;
	void *data;
	size_t element_size;
} Vector;

#define Vector_Npos		(-1)

#define Vector_ForEach(item, vec) \
    for (void *item=vec.data; item<(vec.data+vec.size*vec.element_size); item+=vec.element_size )

extern void Vector_Init(Vector *vec, size_t element_size);
extern void Vector_Clear(Vector *vec);
extern void Vector_Resize(Vector *vec, size_t n);
extern void *Vector_At(Vector *vec, size_t pos);
extern void *Vector_At2(Vector *vec, size_t pos);
extern long Vector_DistanceFromBegin(Vector *vec, const void *it);
extern void *Vector_Front(Vector *vec);
extern void *Vector_Back(Vector *vec);
extern bool Vector_Empty(Vector *vec);
extern void Vector_PushBack(Vector *vec, const void *object);
extern void Vector_PushBack2(Vector *vec, const void *object, size_t len);
extern void *Vector_EmplaceBack(Vector *vec);
extern void *Vector_EmplaceBack2(Vector *vec, size_t len);
extern void Vector_Erase(Vector *vec, const void *object);
extern long Vector_Find(Vector *vec, const void *object);