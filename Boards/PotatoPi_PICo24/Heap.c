#include <stdint.h>
#include <PICo24/Core/IDESupport.h>

#ifndef PICo24_Heap_Size
#define PICo24_Heap_Size	16384
#endif


static uint8_t heap[PICo24_Heap_Size] __attribute__ ((aligned(2)));
void *UMM_Heap_Address = &heap[0];
uint32_t UMM_Heap_Size = PICo24_Heap_Size;


#ifndef PICo24_EDSHeap_Size
#define PICo24_EDSHeap_Size	32767
#endif

uint8_t auto_eds eds_heap[PICo24_EDSHeap_Size] __attribute__((address(0x8000)));
auto_eds void *UMM_EDSHeap_Addr = &eds_heap[0];
uint32_t UMM_EDSHeap_Size = PICo24_EDSHeap_Size;
