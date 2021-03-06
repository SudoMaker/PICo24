/*
 * Configuration for umm_malloc - DO NOT EDIT THIS FILE BY HAND!
 *
 * Refer to the notes below for how to configure the build at compile time
 * using -D to define non-default values
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <PICo24/Core/IDESupport.h>
#include <ScratchLibc/ScratchLibc.h>


#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>


#define UMM_INTEGRITY_CHECK

/*
 * There are a number of defines you can set at compile time that affect how
 * the memory allocator will operate.
 *
 * Unless otherwise noted, the default state of these values is #undef-ined!
 *
 * If you set them via the -D option on the command line (preferred method)
 * then this file handles all the configuration automagically and warns if
 * there is an incompatible configuration.
 *
 * UMM_TEST_BUILD
 *
 * Set this if you want to compile in the test suite
 *
 * UMM_BLOCK_BODY_SIZE
 *
 * Defines the umm_block[].body size - it is 8 by default
 *
 * This assumes umm_ptr is a pair of uint16_t values
 * which is 4 bytes plus the data[] array which is another 4 bytes
 * for a total of 8.
 *
 * NOTE WELL that the umm_block[].body size must be multiple of
 *           the natural access size of the host machine to ensure
 *           that accesses are efficient.
 *
 *           We have not verified the checks below for 64 bit machines
 *           because this library is targeted for 32 bit machines.
 *
 * UMM_BEST_FIT (default)
 *
 * Set this if you want to use a best-fit algorithm for allocating new blocks.
 * On by default, turned off by UMM_FIRST_FIT
 *
 * UMM_FIRST_FIT
 *
 * Set this if you want to use a first-fit algorithm for allocating new blocks.
 * Faster than UMM_BEST_FIT but can result in higher fragmentation.
 *
 * UMM_INFO
 *
 * Set if you want the ability to calculate metrics on demand
 *
 * UMM_INLINE_METRICS
 *
 * Set this if you want to have access to a minimal set of heap metrics that
 * can be used to gauge heap health.
 * Setting this at compile time will automatically set UMM_INFO.
 * Note that enabling this define will add a slight runtime penalty.
 *
 * UMM_INTEGRITY_CHECK
 *
 * Set if you want to be able to verify that the heap is semantically correct
 * before or after any heap operation - all of the block indexes in the heap
 * make sense.
 * Slows execution dramatically but catches errors really quickly.
 *
 * UMM_POISON_CHECK
 *
 * Set if you want to be able to leave a poison buffer around each allocation.
 * Note this uses an extra 8 bytes per allocation, but you get the benefit of
 * being able to detect if your program is writing past an allocated buffer.
 *
 * DBGLOG_ENABLE
 *
 * Set if you want to enable logging - the default is to use printf() but
 * if you have any special requirements such as thread safety or a custom
 * logging routine - you are free to everride the default
 *
 * DBGLOG_LEVEL=n
 *
 * Set n to a value from 0 to 6 depending on how verbose you want the debug
 * log to be
 *
 * ----------------------------------------------------------------------------
 *
 * Support for this library in a multitasking environment is provided when
 * you add bodies to the UMM_CRITICAL_ENTRY and UMM_CRITICAL_EXIT macros
 * (see below)
 *
 * ----------------------------------------------------------------------------
 */

/* A couple of macros to make packing structures less compiler dependent */

#define UMM_H_ATTPACKPRE
#define UMM_H_ATTPACKSUF __attribute__((__packed__))

/* -------------------------------------------------------------------------- */

#ifndef UMM_EDS_BLOCK_BODY_SIZE
#define UMM_EDS_BLOCK_BODY_SIZE (8)
#endif

#define UMM_EDS_MIN_BLOCK_BODY_SIZE (8)

#if (UMM_EDS_BLOCK_BODY_SIZE < UMM_EDS_MIN_BLOCK_BODY_SIZE)
#error UMM_EDS_BLOCK_BODY_SIZE must be at least 8!
#endif

#if ((UMM_EDS_BLOCK_BODY_SIZE % 4) != 0)
#error UMM_EDS_BLOCK_BODY_SIZE must be multiple of 4!
#endif

/* -------------------------------------------------------------------------- */

#ifdef UMM_BEST_FIT
#ifdef  UMM_FIRST_FIT
#error Both UMM_BEST_FIT and UMM_FIRST_FIT are defined - pick one!
#endif
#else /* UMM_BEST_FIT is not defined */
#ifndef UMM_FIRST_FIT
#define UMM_BEST_FIT
#endif
#endif

/* -------------------------------------------------------------------------- */

typedef struct UMM_EDS_HEAP_INFO_t {
	uint32_t totalEntries;
	uint32_t usedEntries;
	uint32_t freeEntries;

	uint32_t totalBlocks;
	uint32_t usedBlocks;
	uint32_t freeBlocks;
	uint32_t freeBlocksSquared;

	uint32_t maxFreeContiguousBlocks;

	uint32_t totalSize, usedSize;

	uint8_t usage_metric;
	uint8_t fragmentation_metric;
} UMM_EDS_HEAP_INFO;

extern void umm_eds_info_show();
extern void umm_eds_info(UMM_EDS_HEAP_INFO *ummHeapInfo);

/*
 * A couple of macros to make it easier to protect the memory allocator
 * in a multitasking system. You should set these macros up to use whatever
 * your system uses for this purpose. You can disable interrupts entirely, or
 * just disable task switching - it's up to you
 *
 * NOTE WELL that these macros MUST be allowed to nest, because umm_free() is
 * called from within umm_malloc()
 */


#define UMM_CRITICAL_ENTRY() taskENTER_CRITICAL()
#define UMM_CRITICAL_EXIT() taskEXIT_CRITICAL()

/*
 * Enables heap integrity check before any heap operation. It affects
 * performance, but does NOT consume extra memory.
 *
 * If integrity violation is detected, the message is printed and user-provided
 * callback is called: `UMM_HEAP_CORRUPTION_CB()`
 *
 * Note that not all buffer overruns are detected: each buffer is aligned by
 * 4 bytes, so there might be some trailing "extra" bytes which are not checked
 * for corruption.
 */

#ifdef UMM_INTEGRITY_CHECK
extern bool umm_integrity_check(void);
#define INTEGRITY_CHECK() umm_integrity_check()
extern void umm_corruption(void);
#define UMM_HEAP_CORRUPTION_CB() printf("Heap Corruption!")
#else
#define INTEGRITY_CHECK() (1)
#endif

/*
 * Add blank macros for DBGLOG_xxx() - if you want to override these on
 * a per-source module basis, you must define DBGLOG_LEVEL and then
 * #include "dbglog.h"
 */

#define UMM_LOG_TRACE(format, ...)
//#define UMM_LOG_DEBUG(format, ...)
#define UMM_LOG_CRITICAL(format, ...)
#define UMM_LOG_ERROR(format, ...)
#define UMM_LOG_WARNING(format, ...)
//#define UMM_LOG_INFO(format, ...)

//#define UMM_LOG_TRACE(format, ...) printf(format, ##__VA_ARGS__)
#define UMM_EDS_LOG_DEBUG(format, ...) printf(format, ##__VA_ARGS__)
//#define UMM_LOG_CRITICAL(format, ...) printf(format, ##__VA_ARGS__)
//#define UMM_LOG_ERROR(format, ...) printf(format, ##__VA_ARGS__)
//#define UMM_LOG_WARNING(format, ...) printf(format, ##__VA_ARGS__)
#define UMM_LOG_INFO(format, ...) printf(format, ##__VA_ARGS__)