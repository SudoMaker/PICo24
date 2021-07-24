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

#include "Variant.h"

uint8_t Variant_AsUint8(void *data) {
	return *(uint8_t *)data;
}

int8_t Variant_AsInt8(void *data) {
	return *(int8_t *)data;
}

// Such PIC24, very align, WOW 🐶
// Such XC16, very not duty, WOW 🌚
// Such MCHP Cust Support, very shouldn't, very WARM❤️, WOW 👺

uint16_t Variant_AsUint16(void *data) {
	uint16_t ret;
	memcpy(&ret, data, sizeof(uint16_t));
	return ret;
}

int16_t Variant_AsInt16(void *data) {
	int16_t ret;
	memcpy(&ret, data, sizeof(int16_t));
	return ret;
}

uint32_t Variant_AsUint32(void *data) {
	uint32_t ret;
	memcpy(&ret, data, sizeof(uint32_t));
	return ret;
}

int32_t Variant_AsInt32(void *data) {
	int32_t ret;
	memcpy(&ret, data, sizeof(int32_t));
	return ret;
}

uint64_t Variant_AsUint64(void *data) {
	uint64_t ret;
	memcpy(&ret, data, sizeof(uint64_t));
	return ret;
}

int64_t Variant_AsInt64(void *data) {
	int64_t ret;
	memcpy(&ret, data, sizeof(int64_t));
	return ret;
}