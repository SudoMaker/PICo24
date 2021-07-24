/* ----------------------------------------------------------------------------
 * umm_malloc.h - a memory allocator for embedded systems (microcontrollers)
 *
 * See copyright notice in LICENSE.TXT
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <stdint.h>

#include "umm_eds_malloc_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

extern auto_eds void *UMM_EDSHeap_Addr;
extern uint32_t UMM_EDSHeap_Size;
extern uint16_t umm_eds_numblocks;

/* ------------------------------------------------------------------------- */

UMM_H_ATTPACKPRE typedef struct umm_eds_ptr_t {
	uint16_t next;
	uint16_t prev;
} UMM_H_ATTPACKSUF umm_eds_ptr;

UMM_H_ATTPACKPRE typedef struct umm_eds_block_t {
	union {
		umm_eds_ptr used;
	} header;
	union {
		umm_eds_ptr free;
		uint8_t data[8 - sizeof(struct umm_eds_ptr_t)];
	} body;
} UMM_H_ATTPACKSUF umm_eds_block;

#define UMM_EDS_FREELIST_MASK ((uint16_t)(0x8000))
#define UMM_EDS_BLOCKNO_MASK  ((uint16_t)(0x7FFF))

#define UMM_EDS_NUMBLOCKS  (umm_eds_numblocks)
#define UMM_EDS_BLOCK_LAST (UMM_EDS_NUMBLOCKS - 1)

/* -------------------------------------------------------------------------
 * These macros evaluate to the address of the block and data respectively
 */

#define UMM_EDS_BLOCK(b)  (umm_eds_heap[b])
#define UMM_EDS_DATA(b)   (UMM_EDS_BLOCK(b).body.data)

/* -------------------------------------------------------------------------
 * These macros evaluate to the index of the block - NOT the address!!!
 */

#define UMM_EDS_NBLOCK(b) (UMM_EDS_BLOCK(b).header.used.next)
#define UMM_EDS_PBLOCK(b) (UMM_EDS_BLOCK(b).header.used.prev)
#define UMM_EDS_NFREE(b)  (UMM_EDS_BLOCK(b).body.free.next)
#define UMM_EDS_PFREE(b)  (UMM_EDS_BLOCK(b).body.free.prev)

/* ------------------------------------------------------------------------ */

extern auto_eds umm_eds_block *umm_eds_heap;

extern void umm_eds_init();
extern auto_eds void *umm_eds_malloc(uint32_t size);
extern auto_eds void *umm_eds_calloc(uint32_t num, uint32_t size);
extern auto_eds void *umm_eds_realloc(auto_eds void *ptr, uint32_t size);
extern void umm_eds_free(auto_eds void *ptr);

/* ------------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif
