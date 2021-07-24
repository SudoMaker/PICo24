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

#include "PICoBootCompanion.h"

#include <PICo24_Board.h>

#include <PICo24/Core/Delay.h>
#include <PICo24/Peripherals/USB/Device/usb_deluxe_device.h>
#include <PICo24/Peripherals/USB/Device/usb_deluxe_device_cdc_acm.h>

#ifdef PICo24_Board_PICoBootCompanion

PICoBootRuntimeEnvironment picoboot_runtime_env;

static const char PICoBoot_Version[] = "v0.01 [Companion]";

static const int8_t flasher_cmd_arg_length_table[] = {
	-1,	// 0: FlasherCommand_Unknown

	1,	// void FlasherCommand_Reset(uint8_t mode)

	0,	// char[32] FlasherCommand_GetBootloaderVersion()
	0,	// char[32] FlasherCommand_GetBoardName()
	0,	// char[32] FlasherCommand_GetBoardManufacturer()
	0,	// char[32] FlasherCommand_GetChipName()
	0,	// char[32] FlasherCommand_GetChipManufacturer()

	4,	// uint32_t FlasherCommand_FlashSetAddr(uint32_t addr)
	2,	// uint16_t FlasherCommand_FlashSetLength(uint16_t len)
	1,	// Result FlasherCommand_FlashErase(uint8_t mode)
	0,	// uint8_t[] FlasherCommand_FlashRead()
	5,	// Result FlasherCommand_FlashWrite4(uint8_t[4], uint8_t crc8)
	9,	// Result FlasherCommand_FlashWrite8(uint8_t[8], uint8_t crc8)
	17,	// Result FlasherCommand_FlashWrite8(uint8_t[16], uint8_t crc8)


	1,	// char[64] FlasherCommand_EnvironmentRead(uint8_t category)
	4,	// Result FlasherCommand_EnvironmentWrite(uint8_t category, uint8_t offset, uint8_t value, uint8_t crc)
	1,	// Result FlasherCommand_EnvironmentSave(uint8_t category)
};

typedef union {
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
} VariantInt;

typedef struct {
	uint8_t current_command;
	uint8_t variable_command_length;

	uint8_t buffer[32];
	uint8_t buffer_pos;

	uint32_t current_address;
	uint16_t current_length;

	uint8_t read_command;

} PICoBootProtocolContext;

static PICoBootProtocolContext protocol_ctx = {0};

static uint8_t pbc_cdc_idx = 0;

static int PICoBoot_CmdBuffer_Push(uint8_t data) {
	protocol_ctx.buffer[protocol_ctx.buffer_pos] = data;
	protocol_ctx.buffer_pos++;
	return protocol_ctx.buffer_pos;
}

static void PICoBoot_CmdBuffer_Clear() {
	protocol_ctx.buffer_pos = 0;
	protocol_ctx.current_command = 0;
}

static void PICoBoot_DoReads() {
	if (!protocol_ctx.read_command)
		return;

	char buf[64];
	size_t len = 32;

	switch (protocol_ctx.read_command) {
		case PICoBoot_FlasherCommand_GetBootloaderVersion:
			strncpy(buf, PICoBoot_Version, 31);
			break;
		case PICoBoot_FlasherCommand_GetBoardName:
			strncpy(buf, PICo24_Board_Name, 31);
			break;
		case PICoBoot_FlasherCommand_GetBoardManufacturer:
			strncpy(buf, PICo24_Board_Manufacturer, 31);
			break;
		case PICoBoot_FlasherCommand_GetChipName:
			strncpy(buf, PICo24_Board_Chip, 31);
			break;
		case PICoBoot_FlasherCommand_GetChipManufacturer:
			strncpy(buf, PICo24_Board_ChipManufacturer, 31);
			break;
		case PICoBoot_FlasherCommand_EnvironmentRead:
			memset(buf, 0, sizeof(buf));
			if (protocol_ctx.buffer[0] == 0) {

			} else {
				memcpy(buf, &picoboot_runtime_env, sizeof(PICoBootRuntimeEnvironment));
			}
			len = 64;
			break;

		default:
			break;
	}

	USBDeluxeDevice_CDC_ACM_Write(USBDeluxe_DeviceGetDriverContext(pbc_cdc_idx)->drv_ctx, (uint8_t *) buf, len);

	protocol_ctx.read_command = 0;
}

static void PICoBoot_CommandInvoke(uint8_t cmd, uint8_t *arg, uint8_t arg_len) {
	VariantInt v;

	if (arg_len <= 8)
		memcpy(&v, arg, arg_len);

	switch (cmd) {
		case PICoBoot_FlasherCommand_Reset:
			picoboot_runtime_env.reboot_target = v.u8;
			PICoBoot_RuntimeEnvironment_Save();
			USBDeviceDetach();
			Delay_Milliseconds(1000);
			asm("reset");
			break;
		case PICoBoot_FlasherCommand_GetBootloaderVersion:
		case PICoBoot_FlasherCommand_GetBoardName:
		case PICoBoot_FlasherCommand_GetBoardManufacturer:
		case PICoBoot_FlasherCommand_GetChipName:
		case PICoBoot_FlasherCommand_GetChipManufacturer:
		case PICoBoot_FlasherCommand_EnvironmentRead:
			protocol_ctx.read_command = cmd;
			break;
		default:
			PICo24_Discard8 = PICoBoot_FlasherResult_EPERM;
			USBDeluxeDevice_CDC_ACM_Write(USBDeluxe_DeviceGetDriverContext(pbc_cdc_idx)->drv_ctx, &PICo24_Discard8, 1);
			break;
	}
}

static void PICoBoot_CDC_ParseIncomingData(const uint8_t *buf, uint16_t len) {
	for (uint16_t i=0; i<len; i++) {
		uint8_t cur_byte = buf[i];

		if (!protocol_ctx.current_command) {
			if (flasher_cmd_arg_length_table[cur_byte] == 0) {
				PICoBoot_CommandInvoke(cur_byte, NULL, 0);
			} else if (flasher_cmd_arg_length_table[cur_byte] == -1) {
				// Do nothing
			} else {
				protocol_ctx.current_command = cur_byte;
			}
		} else {
			if (PICoBoot_CmdBuffer_Push(cur_byte) == flasher_cmd_arg_length_table[protocol_ctx.current_command]) {
				PICoBoot_CommandInvoke(protocol_ctx.current_command, &protocol_ctx.buffer[0], protocol_ctx.buffer_pos);
				PICoBoot_CmdBuffer_Clear();
			}
		}
	}
}


static void CDC_RxDone(void *userp, uint8_t *buf, uint16_t len) {
	PICoBoot_CDC_ParseIncomingData(buf, len);
}

static USBDeluxeDevice_CDC_ACM_IOps pbc_cdc_iops = {
	.RxDone = CDC_RxDone
};

void PICoBootCompanion_Enable() {
	pbc_cdc_idx = USBDeluxe_DeviceFunction_Add_CDC_ACM(NULL, &pbc_cdc_iops);
	PICoBoot_RuntimeEnvironment_Load();
	USBDeluxe_Device_ConfigApply();
}

void PICoBootCompanion_Tasks() {
	static uint8_t cnt;

	if (cnt == 0) {
		PICoBoot_DoReads();
	}

	cnt++;
}

#endif