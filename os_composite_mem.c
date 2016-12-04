#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_composite.h"

/* Memory allocation function */
void *cMemMalloc(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemMalloc(sz = %d)\n", sz);
    #endif

    return malloc(sz);
}

/* Free a prior allocation */
void cMemFree(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemFree(mem = <ptr>)\n");
    #endif

    free(mem);
}

/* Resize an allocation */
void *cMemRealloc(void* mem, int newSize) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemRealloc(mem = <ptr>, newSize = %d)\n", newSize);
    #endif

    return realloc(mem, newSize);
}

/* Return the size of an allocation */
int cMemSize(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemSize(mem = <ptr>)\n");
    #endif
    return sizeof(mem);
}

/* Round up request size to allocation size */
int cMemRoundup(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemRoundup(sz = %d)\n", sz);
    #endif

    return sz;
}

/* Initialize the memory allocator */
int cMemInit(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemInit(pAppData = %p)\n", pAppData);
    #endif

    return SQLITE_OK;
}

/* Deinitialize the memory allocator */
void cMemShutdown(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemShutdown(pAppData = %p)\n", pAppData);
    #endif
}

#endif // SQLITE_OS_OTHER