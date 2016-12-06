
#if SQLITE_OS_OTHER

#include <stdlib.h> /* for malloc() and friends */

#include "os_composite.h"

char* _get_region(void* mem_allocation) {
    return ((char*)mem_allocation) - sizeof(size_t);
}

void* _get_memory(char* region_start) {
    return (void*)(region_start + sizeof(size_t));
}

char* _malloc_region(size_t sz) {
    char* region_start = malloc(sz + sizeof(size_t));
    *((int*)region_start) = sz;
    return region_start;
}

void _free_region(char* region) {
    free(region);
}

size_t _get_memory_size(void* mem_allocation) {
    char* region_start = _get_region(mem_allocation);
    size_t sz = *((size_t*)region_start);
    return sz;
}

/* Memory allocation function */
void* cMemMalloc(int sz) {
    char* region_start = _malloc_region(sz);
    if( region_start == 0 ) return 0;

    return _get_memory(region_start);
}

/* Free a prior allocation */
void cMemFree(void* mem) {
    if( mem == 0 ) return;
    
    char* region = _get_region(mem);
    free(region);
}

/* Resize an allocation */
void* cMemRealloc(void* mem, int newSize) {
    char* old_region = _get_region(mem);
    size_t old_sz = _get_memory_size(mem);

    char* new_region = _malloc_region(newSize);
    if( new_region == 0 ) return 0;

    int i = 0;
    for( i = 0; i < old_sz; i++ )
        new_region[i + sizeof(int)] = old_region[i + sizeof(int)];
    
    _free_region(old_region);

    return _get_memory(new_region);
}

/* Return the size of an allocation */
int cMemSize(void* mem) {
    return (int)_get_memory_size(mem);
}

/* Round up request size to allocation size */
int cMemRoundup(int sz) {
    return sz;
}

/* Initialize the memory allocator */
int cMemInit(void* pAppData) {
    return SQLITE_OK;
}

/* Deinitialize the memory allocator */
void cMemShutdown(void* pAppData) {
}

#endif // SQLITE_OS_OTHER