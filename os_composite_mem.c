
#if SQLITE_OS_OTHER

#include "os_composite.h"

static char* _malloc_region(int sz);
static void _free_region(char* region);

static inline sqlite3_int64 max(sqlite3_int64 a, int b) {
    sqlite3_int64 c = (sqlite3_int64)b;
    return (a > c) ? a : c;
}

static char* _get_region(void* mem_allocation) {
    return ((char*)mem_allocation) - sizeof(int);
}

static void* _get_memory(char* region_start) {
    return (void*)(region_start + sizeof(int));
}

static int _get_memory_size(void* mem_allocation) {
    char* region_start = _get_region(mem_allocation);
    const int sz = *((int*)region_start);
    return sz;
}

#if SQLITE_MEM_USE_MALLOC
    #include <stdlib.h> /* for malloc() and free() */

    static char* _malloc_region(int sz) {
        char* region_start = malloc(sz + sizeof(int));
        *((int*)region_start) = sz;
        composite_mem_app_data.outstanding_memory += sz + sizeof(int);
        composite_mem_app_data.max_memory = max( composite_mem_app_data.outstanding_memory, composite_mem_app_data.max_memory );
        return region_start;
    }

    static void _free_region(char* region) {
        const int sz = *((int*)region);
        composite_mem_app_data.outstanding_memory -= sz + sizeof(int);
        free(region);
    }
#else
    #define MEMORY_ARENA_SIZE (1024*1024*4) //4MB
    static char memory_arena[MEMORY_ARENA_SIZE];

    static char* _free_memory = &memory_arena[0];
    static char* _memory_extent = _free_memory + MEMORY_ARENA_SIZE;

    static char* _malloc_region(int sz) {
        /* make sure that we have enough memory left */
        sqlite3_int64 mem_remaining = (_memory_extent - _free_memory);
        if( _free_memory + sz >= _memory_extent ) return 0;

        char* region_start = _free_memory;
        _free_memory += sz;

        *((int*)region_start) = sz;
        composite_mem_app_data.outstanding_memory += sz + sizeof(int);
        composite_mem_app_data.max_memory = max( composite_mem_app_data.outstanding_memory, composite_mem_app_data.max_memory );
        return region_start;
    }

    static void _free_region(char* region) {
        //Do nothing
    }
#endif

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
