#include <stdint.h>
#include <PICo24/Core/IDESupport.h>

#ifndef PICo24_Heap_Size
#define PICo24_Heap_Size	1024
#endif


static uint8_t heap[PICo24_Heap_Size] __attribute__ ((aligned(2)));
void *UMM_Heap_Address = &heap[0];
uint32_t UMM_Heap_Size = PICo24_Heap_Size;
