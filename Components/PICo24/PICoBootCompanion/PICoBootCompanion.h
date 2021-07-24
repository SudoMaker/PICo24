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
#include <stdint.h>
#include <string.h>

typedef struct {
	uint32_t reset_count;
	uint32_t watchdog_failed_count;
	uint8_t boot_reason;
	uint8_t reboot_target;
} PICoBootRuntimeEnvironment;

enum PICoBootResetAction {
	PICoBoot_ResetAction_RunUserApp,
	PICoBoot_ResetAction_StayInBL,
};

enum PICoBootCommand {
	PICoBoot_FlasherCommand_Reset = 1,

	PICoBoot_FlasherCommand_GetBootloaderVersion,
	PICoBoot_FlasherCommand_GetBoardName,
	PICoBoot_FlasherCommand_GetBoardManufacturer,
	PICoBoot_FlasherCommand_GetChipName,
	PICoBoot_FlasherCommand_GetChipManufacturer,

	PICoBoot_FlasherCommand_FlashSetAddr,
	PICoBoot_FlasherCommand_FlashSetLength,
	PICoBoot_FlasherCommand_FlashErase,
	PICoBoot_FlasherCommand_FlashRead,
	PICoBoot_FlasherCommand_FlashWrite4,
	PICoBoot_FlasherCommand_FlashWrite8,
	PICoBoot_FlasherCommand_FlashWrite16,

	PICoBoot_FlasherCommand_EnvironmentRead,
	PICoBoot_FlasherCommand_EnvironmentWrite,
	PICoBoot_FlasherCommand_EnvironmentSave,
};

enum PICoBootFlashEraseMode {
	PICoBoot_FlashEraseMode_All = 0,
	PICoBoot_FlashEraseMode_App = 1,
};

enum PICoBootResults {
	PICoBoot_FlasherResult_OK = 0,
	PICoBoot_FlasherResult_CRC_Error = 1,
	PICoBoot_FlasherResult_Verify_Error = 2,
	PICoBoot_FlasherResult_EPERM = 3,
	PICoBoot_FlasherResult_ERANGE = 4,
};

enum PICoBootBootReason {
	PICoBoot_BootReason_POR = 0,
	PICoBoot_BootReason_RESET = 1,
	PICoBoot_BootReason_WDT = 2,
};

enum PICoBootRebootTarget {
	PICoBoot_RebootTarget_Default = 0,
	PICoBoot_RebootTarget_Bootloader = 1,
};

extern PICoBootRuntimeEnvironment picoboot_runtime_env;

extern void PICoBootCompanion_Enable();
extern void PICoBootCompanion_Tasks();
