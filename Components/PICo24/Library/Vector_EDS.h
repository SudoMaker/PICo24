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

#include <ScratchLibc/ScratchLibc.h>

#ifdef __HAS_EDS__

typedef struct {
	size_t size;
	auto_eds void *data;
	size_t element_size;
} Vector_EDS;

#define Vector_EDS_Npos		(-1)

#define Vector_EDS_ForEach(item, vec) \
    for (auto_eds void *item=vec.data; item<(vec.data+vec.size*vec.element_size); item+=vec.element_size )

void Vector_EDS_Init(Vector_EDS *vec, size_t element_size);
void Vector_EDS_Clear(Vector_EDS *vec);
void Vector_EDS_Resize(Vector_EDS *vec, size_t n);
auto_eds void *Vector_EDS_At(Vector_EDS *vec, size_t pos);
auto_eds void *Vector_EDS_At2(Vector_EDS *vec, size_t pos);
long Vector_EDS_DistanceFromBegin(Vector_EDS *vec, auto_eds const void *it);
auto_eds void *Vector_EDS_Front(Vector_EDS *vec);
auto_eds void *Vector_EDS_Back(Vector_EDS *vec);
bool Vector_EDS_Empty(Vector_EDS *vec);
auto_eds void *Vector_EDS_EmplaceBack(Vector_EDS *vec);
auto_eds void *Vector_EDS_EmplaceBack2(Vector_EDS *vec, size_t len);
void Vector_EDS_PushBack(Vector_EDS *vec, auto_eds const void *object);
void Vector_EDS_PushBack2(Vector_EDS *vec, auto_eds const void *object, size_t len);
void Vector_EDS_Erase(Vector_EDS *vec, auto_eds const void *object);
long Vector_EDS_Find(Vector_EDS *vec, auto_eds const void *object);

#else

#define Vector_EDS_Npos			Vector_Npos
#define Vector_EDS_ForEach(item, vec)	Vector_ForEach(item, vec)

#define Vector_EDS_Init			Vector_Init
#define Vector_EDS_Clear		Vector_Clear
#define Vector_EDS_Resize		Vector_Resize
#define Vector_EDS_At			Vector_At
#define Vector_EDS_At2			Vector_At2
#define Vector_EDS_DistanceFromBegin	Vector_DistanceFromBegin
#define Vector_EDS_Front		Vector_Front
#define Vector_EDS_Back			Vector_Back
#define Vector_EDS_Empty		Vector_Empty
#define Vector_EDS_EmplaceBack		Vector_EmplaceBack
#define Vector_EDS_EmplaceBack2		Vector_EmplaceBack2
#define Vector_EDS_PushBack		Vector_PushBack
#define Vector_EDS_PushBack2		Vector_PushBack2
#define Vector_EDS_Erase		Vector_Erase
#define Vector_EDS_Find			Vector_Find

#endif
