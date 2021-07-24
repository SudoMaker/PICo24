#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <math.h>

#include "umm_eds_malloc_cfg.h"
#include "umm_eds_malloc.h"

#ifdef __HAS_EDS__

/* ----------------------------------------------------------------------------
 * One of the coolest things about this little library is that it's VERY
 * easy to get debug information about the memory heap by simply iterating
 * through all of the memory blocks.
 *
 * As you go through all the blocks, you can check to see if it's a free
 * block by looking at the high order bit of the next block index. You can
 * also see how big the block is by subtracting the next block index from
 * the current block number.
 *
 * The umm_info function does all of that and makes the results available
 * in the ummHeapInfo structure.
 * ----------------------------------------------------------------------------
 */


void umm_eds_info_show() {
	if (umm_eds_heap == NULL) {
		umm_eds_init();
	}

	UMM_EDS_HEAP_INFO ummHeapInfo;

	uint16_t blockNo = 0;

	/*
	 * Clear out all of the entries in the ummHeapInfo structure before doing
	 * any calculations..
	 */
	memset(&ummHeapInfo, 0, sizeof(ummHeapInfo));

	ummHeapInfo.totalSize = UMM_EDSHeap_Size;

	/* Protect the critical section... */
	UMM_CRITICAL_ENTRY();

	UMM_LOG_INFO("\n");
	UMM_LOG_INFO("+----------+-------+--------+--------+-------+--------+--------+\n");

	UMM_LOG_INFO("|0x%08lx|B %5u|NB %5u|PB %5u|Z %5u|NF %5u|PF %5u|\n",
		      (&UMM_EDS_BLOCK(blockNo)),
		      blockNo,
		      UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK,
		      UMM_EDS_PBLOCK(blockNo),
		      (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK) - blockNo,
		      UMM_EDS_NFREE(blockNo),
		      UMM_PFREE(blockNo));

	/*
	 * Now loop through the block lists, and keep track of the number and size
	 * of used and free blocks. The terminating condition is an nb pointer with
	 * a value of zero...
	 */

	blockNo = UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK;

	while (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK) {
		size_t curBlocks = (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK) - blockNo;

		++ummHeapInfo.totalEntries;
		ummHeapInfo.totalBlocks += curBlocks;

		/* Is this a free block? */

		if (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_FREELIST_MASK) {
			++ummHeapInfo.freeEntries;
			ummHeapInfo.freeBlocks += curBlocks;
			ummHeapInfo.freeBlocksSquared += (curBlocks * curBlocks);

			if (ummHeapInfo.maxFreeContiguousBlocks < curBlocks) {
				ummHeapInfo.maxFreeContiguousBlocks = curBlocks;
			}

			UMM_LOG_INFO("|0x%08lx|B %5u|NB %5u|PB %5u|Z %5u|NF %5u|PF %5u|\n",
				      (&UMM_EDS_BLOCK(blockNo)),
				      blockNo,
				      UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK,
				      UMM_EDS_PBLOCK(blockNo),
				      (uint16_t)curBlocks,
				      UMM_EDS_NFREE(blockNo),
				      UMM_PFREE(blockNo));

			/* Does this block address match the ptr we may be trying to free? */

			if (NULL == &UMM_EDS_BLOCK(blockNo)) {

				/* Release the critical section... */
				UMM_CRITICAL_EXIT();

				return;
			}
		} else {
			++ummHeapInfo.usedEntries;
			ummHeapInfo.usedBlocks += curBlocks;

			UMM_LOG_INFO("|0x%08lx|B %5u|NB %5u|PB %5u|Z %5u|                 |\n",
				      (&UMM_EDS_BLOCK(blockNo)),
				      blockNo,
				      UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK,
				      UMM_EDS_PBLOCK(blockNo),
				      (uint16_t)curBlocks);
		}

		blockNo = UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK;
	}

	/*
	 * The very last block is used as a placeholder to indicate that
	 * there are no more blocks in the heap, so it cannot be used
	 * for anything - at the same time, the size of this block must
	 * ALWAYS be exactly 1 !
	 */

	UMM_LOG_INFO("|0x%08lx|B %5u|NB %5u|PB %5u|Z %5u|NF %5u|PF %5u|\n",
		      (&UMM_EDS_BLOCK(blockNo)),
		      blockNo,
		      UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK,
		      UMM_EDS_PBLOCK(blockNo),
		      UMM_EDS_NUMBLOCKS - blockNo,
		      UMM_EDS_NFREE(blockNo),
		      UMM_PFREE(blockNo));

	/* Release the critical section... */
	UMM_CRITICAL_EXIT();

	UMM_LOG_INFO("+----------+-------+--------+--------+-------+--------+--------+\n");

	UMM_LOG_INFO("Total Entries %5lu    Used Entries %5lu    Free Entries %5lu\n",
		      ummHeapInfo.totalEntries,
		      ummHeapInfo.usedEntries,
		      ummHeapInfo.freeEntries);

	UMM_LOG_INFO("Total Blocks  %5lu    Used Blocks  %5lu    Free Blocks  %5lu\n",
		      ummHeapInfo.totalBlocks,
		      ummHeapInfo.usedBlocks,
		      ummHeapInfo.freeBlocks);

	UMM_LOG_INFO("+--------------------------------------------------------------+\n");

	if (0 == ummHeapInfo.freeBlocks) {
		ummHeapInfo.usedSize = ummHeapInfo.totalSize;
		ummHeapInfo.usage_metric = 100;        // No free blocks!
		ummHeapInfo.fragmentation_metric = 0; // ... so no fragmentation either!
	} else {
		ummHeapInfo.usedSize = (uint32_t)ummHeapInfo.totalSize * ummHeapInfo.usedBlocks / ummHeapInfo.totalBlocks;
		ummHeapInfo.usage_metric = (uint32_t)ummHeapInfo.usedSize * 100 / ummHeapInfo.totalSize;
		ummHeapInfo.fragmentation_metric = 100 - (((uint32_t)(sqrtf(ummHeapInfo.freeBlocksSquared)) * 100) / (ummHeapInfo.freeBlocks));
	}

	UMM_LOG_INFO("Memory Usage: %lu/%lu - %u%%\n", ummHeapInfo.usedSize, ummHeapInfo.totalSize, ummHeapInfo.usage_metric);
	UMM_LOG_INFO("Fragmentation Metric: %u%%\n", ummHeapInfo.fragmentation_metric);

	UMM_LOG_INFO("+--------------------------------------------------------------+\n");
}

void umm_eds_info(UMM_EDS_HEAP_INFO *ummHeapInfo) {
	if (umm_heap == NULL) {
		umm_init();
	}

	uint16_t blockNo = 0;
	memset(ummHeapInfo, 0, sizeof(UMM_EDS_HEAP_INFO));

	ummHeapInfo->totalSize = UMM_Heap_Size;

	UMM_CRITICAL_ENTRY();

	blockNo = UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK;

	while (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK) {
		size_t curBlocks = (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK) - blockNo;

		++ummHeapInfo->totalEntries;
		ummHeapInfo->totalBlocks += curBlocks;

		/* Is this a free block? */

		if (UMM_EDS_NBLOCK(blockNo) & UMM_EDS_FREELIST_MASK) {
			++ummHeapInfo->freeEntries;
			ummHeapInfo->freeBlocks += curBlocks;
			ummHeapInfo->freeBlocksSquared += (curBlocks * curBlocks);

			if (ummHeapInfo->maxFreeContiguousBlocks < curBlocks) {
				ummHeapInfo->maxFreeContiguousBlocks = curBlocks;
			}

			/* Does this block address match the ptr we may be trying to free? */
			if (NULL == &UMM_EDS_BLOCK(blockNo)) {
				/* Release the critical section... */
				UMM_CRITICAL_EXIT();
				return;
			}
		} else {
			++ummHeapInfo->usedEntries;
			ummHeapInfo->usedBlocks += curBlocks;
		}

		blockNo = UMM_EDS_NBLOCK(blockNo) & UMM_EDS_BLOCKNO_MASK;
	}

	/* Release the critical section... */
	UMM_CRITICAL_EXIT();

	if (0 == ummHeapInfo->freeBlocks) {
		ummHeapInfo->usedSize = ummHeapInfo->totalSize;
		ummHeapInfo->usage_metric = 100;        // No free blocks!
		ummHeapInfo->fragmentation_metric = 0; // ... so no fragmentation either!
	} else {
		ummHeapInfo->usedSize = (uint32_t)ummHeapInfo->totalSize * ummHeapInfo->usedBlocks / ummHeapInfo->totalBlocks;
		ummHeapInfo->usage_metric = (uint32_t)ummHeapInfo->usedSize * 100 / ummHeapInfo->totalSize;
		ummHeapInfo->fragmentation_metric = 100 - (((uint32_t)(sqrtf(ummHeapInfo->freeBlocksSquared)) * 100) / (ummHeapInfo->freeBlocks));
	}

}

#endif
