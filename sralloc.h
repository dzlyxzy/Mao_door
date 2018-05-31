/*****************************************************************************
 *
 *              Simple SRAM Dynamic Memory Allocation
 *
 *****************************************************************************
 * This is a simple dynamic memory allocation module. The following are the
 * supported services:
 *
 * BYTE * NEAR SRAMalloc(NEAR BYTE nBytes)
 * void SRAMfree(BYTE * NEAR pSRAM)
 * void SRAMInitHeap(void)
 *
 * This version of the dynamic memory allocation limits the segment size
 * to 126 bytes. This is specifically designed such to enable better
 * performance by limiting pointer manipulation.
 *
 *
 * How it works:
 * The model is based on a simple form of a linked list. A block of memory
 * refered to as the dynamic heap is split into segments. Each segment
 * has a single byte header that references the next segment in the list
 * as well as indicating whether the segment is allocated. Consiquently
 * the reference implicitly identifies the length of the segment.
 *
 * This method also enables the possibility of allowing a large number
 * of memory allocations. The maximum is limited by the defined heap size.
 *
 * SRAMalloc() is used to split or merge segments to be allocated.
 * SRAMfree() is used to release segments.
 *
 * Example:
 *  ----------
 *  |  0x7F  |  0x200   Header Seg1
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |  0x89  |  0x27F   Header Seg2 (allocated)
 *  |        |
 *  |        |
 *  |  0x77  |  0x288   Header Seg3
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |        |
 *  |  0x00  |  0x2FF   Tail
 *  ----------
 *
 *
 *  Bit 7   Bit 6   Bit 5   Bit 4   Bit 3   Bit 2   Bit 1   Bit 0
 *
 *  Alloc   ------------- reference to next Header --------------

 *****************************************************************************/

#ifndef _SRAM_H
#define _SRAM_H

#include "common.h"


/*********************************************************************
 * Segment header data type
 ********************************************************************/
typedef union _SALLOC
{
   WORD byte;		//由于PIC24F单片机是16位的，RAM也是16位的，故内存分区用16位代替
    struct _BITS
    {
        WORD count:7; //是内存块的大小，包含管理信息
        WORD alloc:1; //表明该内存块是否已经分配
		WORD :8;
    }bits;
}SALLOC;


/*********************************************************************
 * Function:        BYTE * SRAMalloc(BYTE length)
 *
 * PreCondition:    A memory block must be allocated in the linker,
 *                  and the memory headers and tail must already be
 *                  set via the function SRAMInitHeap().
 *
 * Input:           BYTE nBytes - Number of bytes to allocate.
 *
 * Output:          BYTE * - A pointer to the requested block
 *                  of memory.
 *
 * Side Effects:
 *
 * Overview:        This functions allocates a chunk of memory from
 *                  the heap. The maximum segment size for this
 *                  version is 126 bytes. If the heap does not have
 *                  an available segment of sufficient size it will
 *                  attempt to create a segment; otherwise a NULL
 *                  pointer is returned. If allocation is succeessful
 *                  then a pointer to the requested block is returned.
 *
 * Note:            The calling function must maintain the pointer
 *                  to correctly free memory at runtime.
 ********************************************************************/
void *SRAMalloc(BYTE nSize);



/*********************************************************************
 * Function:        void SRAMfree(WORD * pSRAM)
 *
 * PreCondition:    The pointer must have been returned from a
 *                  previously allocation via SRAMalloc().
 *
 * Input:           WORD * pSRAM - pointer to the allocated
 *
 * Output:          void
 *
 * Side Effects:
 *
 * Overview:        This function de-allocates a previously allocated
 *                  segment of memory.
 *
 * Note:            The pointer must be a valid pointer returned from
 *                  SRAMalloc(); otherwise, the segment may not be
 *                  successfully de-allocated, and the heap may be
 *                  corrupted.
 ********************************************************************/
void SRAMfree(void * pSRAM);


/*********************************************************************
 * Function:        void SRAMInitHeap(void)
 *
 * PreCondition:
 *
 * Input:           void
 *
 * Output:          void
 *
 * Side Effects:
 *
 * Overview:        This function initializes the dynamic heap. It
 *                  inserts segment headers to maximize segment space.
 *
 * Note:            This function must be called at least one time.
 *                  And it could be called more times to reset the
 *                  heap.
 ********************************************************************/
void SRAMInitHeap(void);

BOOL SRAMmerge(SALLOC *pSegA);

#endif
