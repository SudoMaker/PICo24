/* ----------------------------------------------------------------------------
 * umm_malloc.h - a memory allocator for embedded systems (microcontrollers)
 *
 * See copyright notice in LICENSE.TXT
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <stdint.h>

#include "umm_malloc_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void *UMM_Heap_Address;
extern uint32_t UMM_Heap_Size;
extern uint16_t umm_numblocks;

/* ------------------------------------------------------------------------- */

UMM_H_ATTPACKPRE typedef struct umm_ptr_t {
	uint16_t next;
	uint16_t prev;
} UMM_H_ATTPACKSUF umm_ptr;

UMM_H_ATTPACKPRE typedef struct umm_block_t {
	union {
		umm_ptr used;
	} header;
	union {
		umm_ptr free;
		uint8_t data[UMM_BLOCK_BODY_SIZE - sizeof(struct umm_ptr_t)];
	} body;
} UMM_H_ATTPACKSUF umm_block;

#define UMM_FREELIST_MASK ((uint16_t)(0x8000))
#define UMM_BLOCKNO_MASK  ((uint16_t)(0x7FFF))

#define UMM_NUMBLOCKS  (umm_numblocks)
#define UMM_BLOCK_LAST (UMM_NUMBLOCKS - 1)

/* -------------------------------------------------------------------------
 * These macros evaluate to the address of the block and data respectively
 */

#define UMM_BLOCK(b)  (umm_heap[b])
#define UMM_DATA(b)   (UMM_BLOCK(b).body.data)

/* -------------------------------------------------------------------------
 * These macros evaluate to the index of the block - NOT the address!!!
 */

#define UMM_NBLOCK(b) (UMM_BLOCK(b).header.used.next)
#define UMM_PBLOCK(b) (UMM_BLOCK(b).header.used.prev)
#define UMM_NFREE(b)  (UMM_BLOCK(b).body.free.next)
#define UMM_PFREE(b)  (UMM_BLOCK(b).body.free.prev)

/* ------------------------------------------------------------------------ */

extern umm_block *umm_heap;


extern void  umm_init(void);
extern void *umm_malloc(size_t size);
extern void *umm_calloc(size_t num, size_t size);
extern void *umm_realloc(void *ptr, size_t size);
extern void  umm_free(void *ptr);

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
