/* ----------------------------------------------------------------------------
 * umm_malloc.c - a memory allocator for embedded systems (microcontrollers)
 *
 * See LICENSE for copyright notice
 * See README.md for acknowledgements and description of internals
 * ----------------------------------------------------------------------------
 *
 * R.Hempel 2007-09-22 - Original
 * R.Hempel 2008-12-11 - Added MIT License biolerplate
 *                     - realloc() now looks to see if previous block is free
 *                     - made common operations functions
 * R.Hempel 2009-03-02 - Added macros to disable tasking
 *                     - Added function to dump heap and check for valid free
 *                        pointer
 * R.Hempel 2009-03-09 - Changed name to umm_malloc to avoid conflicts with
 *                        the mm_malloc() library functions
 *                     - Added some test code to assimilate a free block
 *                        with the very block if possible. Complicated and
 *                        not worth the grief.
 * D.Frank 2014-04-02  - Fixed heap configuration when UMM_TEST_MAIN is NOT set,
 *                        added user-dependent configuration file umm_malloc_cfg.h
 * R.Hempel 2016-12-04 - Add support for Unity test framework
 *                     - Reorganize source files to avoid redundant content
 *                     - Move integrity and poison checking to separate file
 * R.Hempel 2017-12-29 - Fix bug in realloc when requesting a new block that
 *                        results in OOM error - see Issue 11
 * R.Hempel 2019-09-07 - Separate the malloc() and free() functionality into
 *                        wrappers that use critical section protection macros
 *                        and static core functions that assume they are
 *                        running in a protected con text. Thanks @devyte
 * R.Hempel 2020-01-07 - Add support for Fragmentation metric - See Issue 14
 * R.Hempel 2020-01-12 - Use explicitly sized values from stdint.h - See Issue 15
 * R.Hempel 2020-01-20 - Move metric functions back to umm_info - See Issue 29
 * R.Hempel 2020-02-01 - Macro functions are uppercased - See Issue 34
 * R.Hempel 2020-06-20 - Support alternate body size - See Issue 42
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "umm_eds_malloc_cfg.h"   /* user-dependent */
#include "umm_eds_malloc.h"

#ifdef __HAS_EDS__

/* Use the default DBGLOG_LEVEL and DBGLOG_FUNCTION */

// #define DBGLOG_ENABLE

#define DBGLOG_LEVEL 0

#ifdef DBGLOG_ENABLE
#include "dbglog/dbglog.h"
#endif

/* ------------------------------------------------------------------------- */

auto_eds umm_eds_block *umm_eds_heap = NULL;
uint16_t umm_eds_numblocks = 0;

static uint16_t umm_eds_blocks(uint32_t size) {

	/*
	 * The calculation of the block size is not too difficult, but there are
	 * a few little things that we need to be mindful of.
	 *
	 * When a block removed from the free list, the space used by the free
	 * pointers is available for data. That's what the first calculation
	 * of size is doing.
	 */

	if (size <= (sizeof(((umm_eds_block *)0)->body))) {
		return 1;
	}

	/*
	 * If it's for more than that, then we need to figure out the number of
	 * additional whole blocks the size of an umm_eds_block are required.
	 */

	size -= (1 + (sizeof(((umm_eds_block *)0)->body)));

	return 2 + size / (sizeof(umm_eds_block));
}

/* ------------------------------------------------------------------------ */
/*
 * Split the block `c` into two blocks: `c` and `c + blocks`.
 *
 * - `new_freemask` should be `0` if `c + blocks` used, or `UMM_EDS_FREELIST_MASK`
 *   otherwise.
 *
 * Note that free pointers are NOT modified by this function.
 */
static void umm_eds_split_block(uint16_t c,
			    uint16_t blocks,
			    uint16_t new_freemask) {

	UMM_EDS_NBLOCK(c + blocks) = (UMM_EDS_NBLOCK(c) & UMM_EDS_BLOCKNO_MASK) | new_freemask;
	UMM_EDS_PBLOCK(c + blocks) = c;

	UMM_EDS_PBLOCK(UMM_EDS_NBLOCK(c) & UMM_EDS_BLOCKNO_MASK) = (c + blocks);
	UMM_EDS_NBLOCK(c) = (c + blocks);
}

/* ------------------------------------------------------------------------ */

static void umm_eds_disconnect_from_free_list(uint16_t c) {
	/* Disconnect this block from the FREE list */

	UMM_EDS_NFREE(UMM_EDS_PFREE(c)) = UMM_EDS_NFREE(c);
	UMM_EDS_PFREE(UMM_EDS_NFREE(c)) = UMM_EDS_PFREE(c);

	/* And clear the free block indicator */

	UMM_EDS_NBLOCK(c) &= (~UMM_EDS_FREELIST_MASK);
}

/* ------------------------------------------------------------------------
 * The umm_assimilate_up() function does not assume that UMM_EDS_NBLOCK(c)
 * has the UMM_EDS_FREELIST_MASK bit set. It only assimilates up if the
 * next block is free.
 */

static void umm_eds_assimilate_up(uint16_t c) {

	if (UMM_EDS_NBLOCK(UMM_EDS_NBLOCK(c)) & UMM_EDS_FREELIST_MASK) {

		/*
		 * The next block is a free block, so assimilate up and remove it from
		 * the free list
		 */

		UMM_EDS_LOG_DEBUG("umm_eds: Assimilate up to next block, which is FREE\n");

		/* Disconnect the next block from the FREE list */

		umm_eds_disconnect_from_free_list(UMM_EDS_NBLOCK(c));

		/* Assimilate the next block with this one */

		UMM_EDS_PBLOCK(UMM_EDS_NBLOCK(UMM_EDS_NBLOCK(c)) & UMM_EDS_BLOCKNO_MASK) = c;
		UMM_EDS_NBLOCK(c) = UMM_EDS_NBLOCK(UMM_EDS_NBLOCK(c)) & UMM_EDS_BLOCKNO_MASK;
	}
}

/* ------------------------------------------------------------------------
 * The umm_assimilate_down() function assumes that UMM_EDS_NBLOCK(c) does NOT
 * have the UMM_EDS_FREELIST_MASK bit set. In other words, try to assimilate
 * up before assimilating down.
 */

static uint16_t umm_eds_assimilate_down(uint16_t c, uint16_t freemask) {

	// We are going to assimilate down to the previous block because
	// it was free, so remove it from the fragmentation metric

	UMM_EDS_NBLOCK(UMM_EDS_PBLOCK(c)) = UMM_EDS_NBLOCK(c) | freemask;
	UMM_EDS_PBLOCK(UMM_EDS_NBLOCK(c)) = UMM_EDS_PBLOCK(c);

	if (freemask) {
		// We are going to free the entire assimilated block
		// so add it to the fragmentation metric. A good
		// compiler will optimize away the empty if statement
		// when UMM_INFO is not defined, so don't worry about
		// guarding it.

	}

	return UMM_EDS_PBLOCK(c);
}

/* ------------------------------------------------------------------------- */

void umm_eds_init() {
	/* init heap pointer and size, and memset it to 0 */
	umm_eds_heap = (auto_eds umm_eds_block *)UMM_EDSHeap_Addr;
	uint16_t blksize = sizeof(umm_eds_block);
	umm_eds_numblocks = UMM_EDSHeap_Size / blksize;
	printf("umm_eds: init: heap=0x%08lx, heap_size=%lu, block_size=%u, numblocks=%u\n", umm_eds_heap, UMM_EDSHeap_Size, blksize, umm_eds_numblocks);
	memset_eds(umm_eds_heap, 0x00, UMM_Heap_Size);

	/* setup initial blank heap structure */

	/* Set up umm_block[0], which just points to umm_block[1] */
	UMM_EDS_NBLOCK(0) = 1;
	UMM_EDS_NFREE(0) = 1;
	UMM_EDS_PFREE(0) = 1;

	/*
	 * Now, we need to set the whole heap space as a huge free block. We should
	 * not touch umm_block[0], since it's special: umm_block[0] is the head of
	 * the free block list. It's a part of the heap invariant.
	 *
	 * See the detailed explanation at the beginning of the file.
	 *
	 * umm_block[1] has pointers:
	 *
	 * - next `umm_block`: the last one umm_block[n]
	 * - prev `umm_block`: umm_block[0]
	 *
	 * Plus, it's a free `umm_block`, so we need to apply `UMM_EDS_FREELIST_MASK`
	 *
	 * And it's the last free block, so the next free block is 0 which marks
	 * the end of the list. The previous block and free block pointer are 0
	 * too, there is no need to initialize these values due to the init code
	 * that memsets the entire umm_ space to 0.
	 */
	UMM_EDS_NBLOCK(1) = UMM_EDS_BLOCK_LAST | UMM_EDS_FREELIST_MASK;

	/*
	 * Last umm_block[n] has the next block index at 0, meaning it's
	 * the end of the list, and the previous block is umm_block[1].
	 *
	 * The last block is a special block and can never be part of the
	 * free list, so its pointers are left at 0 too.
	 */

	UMM_EDS_PBLOCK(UMM_EDS_BLOCK_LAST) = 1;

// DBGLOG_FORCE(true, "nblock(0) %04x pblock(0) %04x nfree(0) %04x pfree(0) %04x\n", UMM_EDS_NBLOCK(0) & UMM_EDS_BLOCKNO_MASK, UMM_EDS_PBLOCK(0), UMM_EDS_NFREE(0), UMM_EDS_PFREE(0));
// DBGLOG_FORCE(true, "nblock(1) %04x pblock(1) %04x nfree(1) %04x pfree(1) %04x\n", UMM_EDS_NBLOCK(1) & UMM_EDS_BLOCKNO_MASK, UMM_EDS_PBLOCK(1), UMM_EDS_NFREE(1), UMM_EDS_PFREE(1));

	printf("umm_eds: init done\n");

}

/* ------------------------------------------------------------------------
 * Must be called only from within critical sections guarded by
 * UMM_CRITICAL_ENTRY() and UMM_CRITICAL_EXIT().
 */

static void umm_eds_free_core(auto_eds void *ptr) {

	uint16_t c;

	/*
	 * FIXME: At some point it might be a good idea to add a check to make sure
	 *        that the pointer we're being asked to free up is actually within
	 *        the umm_heap!
	 *
	 * NOTE:  See the new umm_info() function that you can use to see if a ptr is
	 *        on the free list!
	 */

	/* Figure out which block we're in. Note the use of truncated division... */

	c = (((auto_eds void *)ptr) - (auto_eds void *)(&(umm_eds_heap[0]))) / sizeof(umm_block);

	UMM_EDS_LOG_DEBUG("umm_eds: Freeing block %6u\n", c);

	/* Now let's assimilate this block with the next one if possible. */

	umm_eds_assimilate_up(c);

	/* Then assimilate with the previous block if possible */

	if (UMM_EDS_NBLOCK(UMM_EDS_PBLOCK(c)) & UMM_EDS_FREELIST_MASK) {

		UMM_EDS_LOG_DEBUG("umm_eds: Assimilate down to previous block, which is FREE\n");

		c = umm_eds_assimilate_down(c, UMM_EDS_FREELIST_MASK);
	} else {
		/*
		 * The previous block is not a free block, so add this one to the head
		 * of the free list
		 */

		UMM_EDS_LOG_DEBUG("umm_eds: Just add to head of free list\n");

		UMM_EDS_PFREE(UMM_EDS_NFREE(0)) = c;
		UMM_EDS_NFREE(c) = UMM_EDS_NFREE(0);
		UMM_EDS_PFREE(c)            = 0;
		UMM_EDS_NFREE(0) = c;

		UMM_EDS_NBLOCK(c) |= UMM_EDS_FREELIST_MASK;
	}
}

/* ------------------------------------------------------------------------ */

void umm_eds_free(auto_eds void *ptr) {

	if (umm_eds_heap == NULL) {
		umm_eds_init();
	}

	/* If we're being asked to free a NULL pointer, well that's just silly! */

	if ((auto_eds void *)0 == ptr) {
		UMM_EDS_LOG_DEBUG("umm_eds: free a null pointer -> do nothing\n");

		return;
	}

	/* Free the memory withing a protected critical section */

	UMM_CRITICAL_ENTRY();

	umm_eds_free_core(ptr);

	UMM_CRITICAL_EXIT();
}

/* ------------------------------------------------------------------------
 * Must be called only from within critical sections guarded by
 * UMM_CRITICAL_ENTRY() and UMM_CRITICAL_EXIT().
 */

static auto_eds void *umm_eds_malloc_core(size_t size) {
	uint16_t blocks;
	uint16_t blockSize = 0;

	uint16_t bestSize;
	uint16_t bestBlock;

	uint16_t cf;

	blocks = umm_eds_blocks(size);

	/*
	 * Now we can scan through the free list until we find a space that's big
	 * enough to hold the number of blocks we need.
	 *
	 * This part may be customized to be a best-fit, worst-fit, or first-fit
	 * algorithm
	 */

	cf = UMM_EDS_NFREE(0);

	bestBlock = UMM_EDS_NFREE(0);
	bestSize = 0x7FFF;

	while (cf) {
		blockSize = (UMM_EDS_NBLOCK(cf) & UMM_EDS_BLOCKNO_MASK) - cf;

		UMM_LOG_TRACE("umm_eds: Looking at block %6u size %6i\n", cf, blockSize);

#if defined UMM_BEST_FIT
		if ((blockSize >= blocks) && (blockSize < bestSize)) {
			bestBlock = cf;
			bestSize = blockSize;
		}
#elif defined UMM_FIRST_FIT
		/* This is the first block that fits! */
        if ((blockSize >= blocks)) {
            break;
        }
        #else
        #error "No UMM_*_FIT is defined - check umm_malloc_cfg.h"
#endif

		cf = UMM_EDS_NFREE(cf);
	}

	if (0x7FFF != bestSize) {
		cf = bestBlock;
		blockSize = bestSize;
	}

	if (UMM_EDS_NBLOCK(cf) & UMM_EDS_BLOCKNO_MASK && blockSize >= blocks) {

		/*
		 * This is an existing block in the memory heap, we just need to split off
		 * what we need, unlink it from the free list and mark it as in use, and
		 * link the rest of the block back into the freelist as if it was a new
		 * block on the free list...
		 */

		if (blockSize == blocks) {
			/* It's an exact fit and we don't neet to split off a block. */
			UMM_EDS_LOG_DEBUG("umm_eds: Allocating %6i blocks starting at %6u - exact\n", blocks, cf);

			/* Disconnect this block from the FREE list */

			umm_eds_disconnect_from_free_list(cf);
		} else {

			/* It's not an exact fit and we need to split off a block. */
			UMM_EDS_LOG_DEBUG("umm_eds: Allocating %6i blocks starting at %6u - existing\n", blocks, cf);

			/*
			 * split current free block `cf` into two blocks. The first one will be
			 * returned to user, so it's not free, and the second one will be free.
			 */
			umm_eds_split_block(cf, blocks, UMM_EDS_FREELIST_MASK /*new block is free*/);

			/*
			 * `umm_split_block()` does not update the free pointers (it affects
			 * only free flags), but effectively we've just moved beginning of the
			 * free block from `cf` to `cf + blocks`. So we have to adjust pointers
			 * to and from adjacent free blocks.
			 */

			/* previous free block */
			UMM_EDS_NFREE(UMM_EDS_PFREE(cf)) = cf + blocks;
			UMM_EDS_PFREE(cf + blocks) = UMM_EDS_PFREE(cf);

			/* next free block */
			UMM_EDS_PFREE(UMM_EDS_NFREE(cf)) = cf + blocks;
			UMM_EDS_NFREE(cf + blocks) = UMM_EDS_NFREE(cf);
		}

	} else {
		/* Out of memory */

		UMM_EDS_LOG_DEBUG("umm_eds: Can't allocate %5u blocks\n", blocks);

		return (auto_eds void *)NULL;
	}

	return (auto_eds void *)&UMM_EDS_DATA(cf);
}

/* ------------------------------------------------------------------------ */

auto_eds void *umm_eds_malloc(uint32_t size) {

	auto_eds void *ptr = NULL;

	if (umm_eds_heap == NULL) {
		umm_eds_init();
	}

	/*
	 * the very first thing we do is figure out if we're being asked to allocate
	 * a size of 0 - and if we are we'll simply return a null pointer. if not
	 * then reduce the size by 1 byte so that the subsequent calculations on
	 * the number of blocks to allocate are easier...
	 */

	if (0 == size) {
		UMM_EDS_LOG_DEBUG("umm_eds: malloc a block of 0 bytes -> do nothing\n");

		return ptr;
	}

	/* Allocate the memory withing a protected critical section */

	UMM_CRITICAL_ENTRY();

	ptr = umm_eds_malloc_core(size);

	UMM_CRITICAL_EXIT();

	return ptr;
}

/* ------------------------------------------------------------------------ */

auto_eds void *umm_eds_realloc(auto_eds void *ptr, uint32_t size) {

	uint16_t blocks;
	uint16_t blockSize;
	uint16_t prevBlockSize = 0;
	uint16_t nextBlockSize = 0;

	uint16_t c;

	uint32_t curSize;

	if (umm_eds_heap == NULL) {
		umm_eds_init();
	}

	/*
	 * This code looks after the case of a NULL value for ptr. The ANSI C
	 * standard says that if ptr is NULL and size is non-zero, then we've
	 * got to work the same a malloc(). If size is also 0, then our version
	 * of malloc() returns a NULL pointer, which is OK as far as the ANSI C
	 * standard is concerned.
	 */

	if (((auto_eds void *)NULL == ptr)) {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc the NULL pointer - call malloc()\n");

		return umm_eds_malloc(size);
	}

	/*
	 * Now we're sure that we have a non_NULL ptr, but we're not sure what
	 * we should do with it. If the size is 0, then the ANSI C standard says that
	 * we should operate the same as free.
	 */

	if (0 == size) {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc to 0 size, just free the block\n");

		umm_eds_free(ptr);

		return (auto_eds void *)NULL;
	}

	/*
	 * Otherwise we need to actually do a reallocation. A naiive approach
	 * would be to malloc() a new block of the correct size, copy the old data
	 * to the new block, and then free the old block.
	 *
	 * While this will work, we end up doing a lot of possibly unnecessary
	 * copying. So first, let's figure out how many blocks we'll need.
	 */

	blocks = umm_eds_blocks(size);

	/* Figure out which block we're in. Note the use of truncated division... */

	c = (((auto_eds void *)ptr) - (auto_eds void *)(&(umm_heap[0]))) / sizeof(umm_block);

	/* Figure out how big this block is ... the free bit is not set :-) */

	blockSize = (UMM_EDS_NBLOCK(c) - c);

	/* Figure out how many bytes are in this block */

	curSize = (blockSize * sizeof(umm_block)) - (sizeof(((umm_eds_block *)0)->header));

	/* Protect the critical section... */
	UMM_CRITICAL_ENTRY();

	/* Now figure out if the previous and/or next blocks are free as well as
	 * their sizes - this will help us to minimize special code later when we
	 * decide if it's possible to use the adjacent blocks.
	 *
	 * We set prevBlockSize and nextBlockSize to non-zero values ONLY if they
	 * are free!
	 */

	if ((UMM_EDS_NBLOCK(UMM_EDS_NBLOCK(c)) & UMM_EDS_FREELIST_MASK)) {
		nextBlockSize = (UMM_EDS_NBLOCK(UMM_EDS_NBLOCK(c)) & UMM_EDS_BLOCKNO_MASK) - UMM_EDS_NBLOCK(c);
	}

	if ((UMM_EDS_NBLOCK(UMM_EDS_PBLOCK(c)) & UMM_EDS_FREELIST_MASK)) {
		prevBlockSize = (c - UMM_EDS_PBLOCK(c));
	}

	UMM_EDS_LOG_DEBUG("umm_eds: realloc blocks %u blockSize %u nextBlockSize %u prevBlockSize %u\n", blocks, blockSize, nextBlockSize, prevBlockSize);

	/*
	 * Ok, now that we're here we know how many blocks we want and the current
	 * blockSize. The prevBlockSize and nextBlockSize are set and we can figure
	 * out the best strategy for the new allocation as follows:
	 *
	 * 1. If the new block is the same size or smaller than the current block do
	 *    nothing.
	 * 2. If the next block is free and adding it to the current block gives us
	 *    EXACTLY enough memory, assimilate the next block. This avoids unwanted
	 *    fragmentation of free memory.
	 *
	 * The following cases may be better handled with memory copies to reduce
	 * fragmentation
	 *
	 * 3. If the previous block is NOT free and the next block is free and
	 *    adding it to the current block gives us enough memory, assimilate
	 *    the next block. This may introduce a bit of fragmentation.
	 * 4. If the prev block is free and adding it to the current block gives us
	 *    enough memory, remove the previous block from the free list, assimilate
	 *    it, copy to the new block.
	 * 5. If the prev and next blocks are free and adding them to the current
	 *    block gives us enough memory, assimilate the next block, remove the
	 *    previous block from the free list, assimilate it, copy to the new block.
	 * 6. Otherwise try to allocate an entirely new block of memory. If the
	 *    allocation works free the old block and return the new pointer. If
	 *    the allocation fails, return NULL and leave the old block intact.
	 *
	 * TODO: Add some conditional code to optimise for less fragmentation
	 *       by simply allocating new memory if we need to copy anyways.
	 *
	 * All that's left to do is decide if the fit was exact or not. If the fit
	 * was not exact, then split the memory block so that we use only the requested
	 * number of blocks and add what's left to the free list.
	 */

	//  Case 1 - block is same size or smaller
	if (blockSize >= blocks) {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc the same or smaller size block - %u, do nothing\n", blocks);
		/* This space intentionally left blank */

		//  Case 2 - block + next block fits EXACTLY
	} else if ((blockSize + nextBlockSize) == blocks) {
		UMM_EDS_LOG_DEBUG("umm_eds: exact realloc using next block - %u\n", blocks);
		umm_eds_assimilate_up(c);
		blockSize += nextBlockSize;

		//  Case 3 - prev block NOT free and block + next block fits
	} else if ((0 == prevBlockSize) && (blockSize + nextBlockSize) >= blocks) {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc using next block - %u\n", blocks);
		umm_eds_assimilate_up(c);
		blockSize += nextBlockSize;

		//  Case 4 - prev block + block fits
	} else if ((prevBlockSize + blockSize) >= blocks) {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc using prev block - %u\n", blocks);
		umm_eds_disconnect_from_free_list(UMM_EDS_PBLOCK(c));
		c = umm_eds_assimilate_down(c, 0);
		memmove_eds((auto_eds void *)&UMM_EDS_DATA(c), ptr, curSize);
		ptr = (auto_eds void *)&UMM_EDS_DATA(c);
		blockSize += prevBlockSize;

		//  Case 5 - prev block + block + next block fits
	} else if ((prevBlockSize + blockSize + nextBlockSize) >= blocks) {
		UMM_EDS_LOG_DEBUG("realloc using prev and next block - %u\n", blocks);
		umm_eds_assimilate_up(c);
		umm_eds_disconnect_from_free_list(UMM_EDS_PBLOCK(c));
		c = umm_eds_assimilate_down(c, 0);
		memmove_eds((auto_eds void *)&UMM_EDS_DATA(c), ptr, curSize);
		ptr = (auto_eds void *)&UMM_EDS_DATA(c);
		blockSize += (prevBlockSize + nextBlockSize);

		//  Case 6 - default is we need to realloc a new block
	} else {
		UMM_EDS_LOG_DEBUG("umm_eds: realloc a completely new block %u\n", blocks);
		auto_eds void *oldptr = ptr;
		if ((ptr = umm_eds_malloc_core(size))) {
			UMM_EDS_LOG_DEBUG("umm_eds: realloc %u to a bigger block %u, copy, and free the old\n", blockSize, blocks);
			memcpy_eds(ptr, oldptr, curSize);
			umm_eds_free_core(oldptr);
		} else {
			UMM_EDS_LOG_DEBUG("umm_eds: realloc %u to a bigger block %u failed - return NULL and leave the old block!\n", blockSize, blocks);
			/* This space intentionally left blnk */
		}
		blockSize = blocks;
	}

	/* Now all we need to do is figure out if the block fit exactly or if we
	 * need to split and free ...
	 */

	if (blockSize > blocks) {
		UMM_EDS_LOG_DEBUG("umm_eds: split and free %u blocks from %u\n", blocks, blockSize);
		umm_eds_split_block(c, blocks, 0);
		umm_eds_free_core((auto_eds void *)&UMM_EDS_DATA(c + blocks));
	}

	/* Release the critical section... */
	UMM_CRITICAL_EXIT();

	return ptr;
}

/* ------------------------------------------------------------------------ */

auto_eds void *umm_eds_calloc(uint32_t num, uint32_t item_size) {
	auto_eds void *ret;

	ret = umm_eds_malloc((uint32_t)(item_size * num));

	if (ret) {
		memset_eds(ret, 0x00, (uint32_t)(item_size * num));
	}

	return ret;
}

/* ------------------------------------------------------------------------ */

#endif
