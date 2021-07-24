#pragma once

extern int freertos_started;

#ifdef PICo24_FreeRTOS_Enabled

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#define PICo24_YIELD()		if (freertos_started) taskYIELD()
#else
#define PICo24_YIELD()
#endif