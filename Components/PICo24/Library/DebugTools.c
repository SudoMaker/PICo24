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

#include "DebugTools.h"

void hexdump(const uint8_t *buf, size_t len) {
	uint8_t cbuf_raw[21] = {0};
	cbuf_raw[0] = ' ';
	cbuf_raw[1] = '|';
	cbuf_raw[18] = '|';
	cbuf_raw[19] = '\n';

	uint8_t* cbuf = &cbuf_raw[2];

	uint8_t last_mod = 0;

	for (size_t i=0; i<len; i++) {
		last_mod = i % 16;
		if (last_mod == 0) {
			printf("%08x  ", i);
		}

		uint8_t p = buf[i];

		if (last_mod == 7) {
			printf("%02x  ", p);
		} else {
			printf("%02x ", p);
		}

		if (isprint(p)) {
			cbuf[last_mod] = p;
		} else {
			cbuf[last_mod] = '.';
		}

		if (last_mod == 15) {
			printf("%s", cbuf_raw);
		}
	}

	if (len && last_mod - 15) {
		cbuf[last_mod + 1] = '|';
		cbuf[last_mod + 2] = '\n';
		cbuf[last_mod + 3] = 0;
		if (last_mod < 7) {
			printf(" ");
		}
		for (size_t i = last_mod + 1; i < 16; i += 1) {
			printf("   ");
		}

		printf("%s", cbuf_raw);


	}

	printf("%08x\n", len);
}

#ifdef __HAS_EDS__

void hexdump_eds(const auto_eds uint8_t *buf, uint32_t len) {
	uint8_t cbuf_raw[21] = {0};
	cbuf_raw[0] = ' ';
	cbuf_raw[1] = '|';
	cbuf_raw[18] = '|';
	cbuf_raw[19] = '\n';

	uint8_t* cbuf = &cbuf_raw[2];

	uint8_t last_mod = 0;

	for (uint32_t i=0; i<len; i++) {
		last_mod = i % 16;
		if (last_mod == 0) {
			printf("%08lx  ", i);
		}

		uint8_t p = buf[i];

		if (last_mod == 7) {
			printf("%02x  ", p);
		} else {
			printf("%02x ", p);
		}

		if (isprint(p)) {
			cbuf[last_mod] = p;
		} else {
			cbuf[last_mod] = '.';
		}

		if (last_mod == 15) {
			printf("%s", cbuf_raw);
		}
	}

	if (len && last_mod - 15) {
		cbuf[last_mod + 1] = '|';
		cbuf[last_mod + 2] = '\n';
		cbuf[last_mod + 3] = 0;
		if (last_mod < 7) {
			printf(" ");
		}
		for (size_t i = last_mod + 1; i < 16; i += 1) {
			printf("   ");
		}

		printf("%s", cbuf_raw);


	}

	printf("%08lx\n", len);
}

#endif
