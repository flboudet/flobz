#include "ios_memory.h"

#ifdef DEBUG_MEMORY

void * memoryBlock[NB_MALLOC_MAX];
int    memoryBSize[NB_MALLOC_MAX];
int    nbMalloc;

int    findMemoryBlock(const void *ptr) {
    for (int i=nbMalloc-1; i>=0; --i)
        if (ptr == memoryBlock[i]) return i;
    return -1;
}
void   addMemoryBlock(void *ptr, int size) {
    if (nbMalloc >= NB_MALLOC_MAX) IOS_ERROR("TO MUCH MALLOC FOR DEBUGGING!");
    memoryBlock[nbMalloc] = ptr;
    memoryBSize[nbMalloc] = size;
    nbMalloc++;
}

void   removeMemoryBlock(void *ptr) {
    int i = findMemoryBlock(ptr);
    if (i<0) IOS_ERROR("COULD NOT FREE MEMORY BLOCK, UNKNOWN BLOCK");
    if (nbMalloc<=0) IOS_ERROR("NOTHING TO FREE: NO MORE MEMORY BLOCKS");
    nbMalloc--;
    memoryBlock[i]=memoryBlock[nbMalloc];
    memoryBSize[i]=memoryBSize[nbMalloc];
}

#endif
