/*
* $Id: realloc.cpp,v 1.13 2002-08-25 19:31:39 jlatone Exp $
*/

/*---------+
| Includes |
+---------*/
#include <stddef.h>
#include <new.h>
#include <assert.h>

#include "realloc.h"

/*----------------------------------------------------------reallocFix4Mwerks-+
| A MW (Ben Combee) replacement for this function in <cstdlib_Palm_OS>        |
| in MSL C++                                                                  |
|       __inline void *realloc(void *ptr, size_t size)                        |
| which wasn't checking for memErrChunkLocked.                                |
+----------------------------------------------------------------------------*/
/*__inline*/ void * reallocFix4Mwerks(void *ptr, size_t size)
{
	Err err;
	
	// try some special tricks for 3.x only
    static UInt32 romversion = 0;
    static bool is3x = false;
    if (romversion == 0) {
    	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romversion);
    	is3x = (romversion <= sysMakeROMVersion(3, 3, 0, sysROMStageRelease, 0));
    }

    // input ptr NULL is same as malloc
    if (ptr == 0) { return MemPtrNew(size); }

    // MemPtrNew(0) always returns NULL
    if (size == 0) { MemPtrFree(ptr); return 0; }

    // if we can't resize in place, alloc new block.
	// if we're 3.x, then for larger reallocs release the old memory first
	// before trying to successfully allocate it.  
	// this will hopefully lead to less fragmentation.
	Int32 oldSize = MemPtrSize(ptr);
    err = (is3x && size > oldSize) ? memErrChunkLocked : MemPtrResize(ptr, size);
    if (err == 0) { return ptr; }
    if (err == memErrChunkLocked) {    
    
        void * newPtr = (is3x && size > oldSize) ? 0 : MemPtrNew(size);
        if (newPtr) {
	        MemMove(newPtr, ptr, oldSize);
    	    MemPtrFree(ptr);
        	return newPtr;
        }
		        
        ///////////////////////////////////////////////////////////
		
		// If we're really out of heap space,
		// then we could at least try to copy the old chunk to a db
		// and then free it before we try to reallocate new space.

		// Copy the old data to it.
     	MemHandle hMemory = DmNewHandle(0, oldSize);
     	if (!hMemory) { throw std::bad_alloc(); }
	 	char * pMemory = (char *)MemHandleLock(hMemory);
     	if (!pMemory) { throw std::bad_alloc(); }
     	if (DmWriteCheck(pMemory, 0, oldSize) != errNone) {
	 		MemHandleUnlock(hMemory);
	 		throw std::bad_alloc(); 
     	}
	 	if ((err = DmWrite(pMemory, 0, ptr, oldSize)) != errNone) { 
	 		MemHandleUnlock(hMemory);
	 		throw std::bad_alloc(); 
	 	}
	 	
	 	// Free the old chunk.
	 	UInt16 heap = MemPtrHeapID(ptr);
	 	err = MemPtrFree(ptr);
//	 	static UInt32 freeBytes, maxChunk; 
//	 	MemHeapFreeBytes(heap, &freeBytes, &maxChunk);
//		if (size > maxChunk) {
//			MemHeapCompact(heap);
//		 	MemHeapFreeBytes(heap, &freeBytes, &maxChunk);
//			if (size > maxChunk) WE'RE IN TROUBLE!
//		}
	 	
	 	// Try to get a new chunk.
        newPtr = MemPtrNew(size);
        if (!newPtr) { 
	 		MemHandleUnlock(hMemory);
	 		throw std::bad_alloc(); 
	 	}
	 	
	 	// Copy the old data to the new chunk.
	 	MemMove(newPtr, pMemory, oldSize);
	 	MemHandleUnlock(hMemory);
	 	
	 	return newPtr;
   
        ///////////////////////////////////////////////////////////
        
    }
    /*
    | any other error condition requires freeing original pointer
    | and returning NULL
    */
    MemPtrFree(ptr);
    return 0;
}

/*===========================================================================*/
