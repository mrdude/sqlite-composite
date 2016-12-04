#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_composite.h"

/* Memory allocation function */
void* cMemMalloc(int sz) {
    return malloc(sz);
}

/* Free a prior allocation */
void cMemFree(void* mem) {
    

    free(mem);
}

/* Resize an allocation */
void* cMemRealloc(void* mem, int newSize) {
    return realloc(mem, newSize);
}

/* Return the size of an allocation */
int cMemSize(void* mem) {
    return sizeof(mem);
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