#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_composite.h"

int cMutexInit() {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexInit()\n");
    #endif
}

int cMutexEnd() {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexEnd()\n");
    #endif
}

/* creates a mutex of the given type
 * @param mutexType one of SQLITE_MUTEX_*
 * @return a pointer to a mutex, or NULL if it couldn't be created
 */
sqlite3_mutex* cMutexAlloc(int mutexType) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexAlloc(mutexType = ");
    switch( mutexType ) {
        case SQLITE_MUTEX_FAST: printf(" MUTEX_FAST "); break;
        case SQLITE_MUTEX_RECURSIVE: printf(" MUTEX_RECURSIVE "); break;
        case SQLITE_MUTEX_STATIC_MASTER: printf(" MUTEX_STATIC_MASTER "); break;
        case SQLITE_MUTEX_STATIC_MEM: printf(" MUTEX_STATIC_MEM "); break;
        case SQLITE_MUTEX_STATIC_OPEN: printf(" MUTEX_STATIC_OPEN "); break;
        case SQLITE_MUTEX_STATIC_PRNG: printf(" MUTEX_STATIC_PRNG "); break;
        case SQLITE_MUTEX_STATIC_LRU: printf(" MUTEX_STATIC_LRU "); break;
        case SQLITE_MUTEX_STATIC_PMEM: printf(" MUTEX_STATIC_PMEM "); break;
        case SQLITE_MUTEX_STATIC_APP1: printf(" MUTEX_STATIC_APP1 "); break;
        case SQLITE_MUTEX_STATIC_APP2: printf(" MUTEX_STATIC_APP2 "); break;
        case SQLITE_MUTEX_STATIC_APP3: printf(" MUTEX_STATIC_APP3 "); break;
        case SQLITE_MUTEX_STATIC_VFS1: printf(" MUTEX_STATIC_VFS1 "); break;
        case SQLITE_MUTEX_STATIC_VFS2: printf(" MUTEX_STATIC_VFS2 "); break;
        case SQLITE_MUTEX_STATIC_VFS3: printf(" MUTEX_STATIC_VFS3 "); break; 
    }
    printf(")\n");
    #endif

}

void cMutexFree(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexFree(mutex)\n");
    #endif

}

/* tries to enter the given mutex.
 * if another thread is in the mutex, this method will block.
 */
void cMutexEnter(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexEnter(mutex)\n");
    #endif

}

/* tries to enter the given mutex.
 * if another thread is in the mutex, this method will return SQLITE_BUSY.
 *
 * "Some systems ... do not support the operation implemented by sqlite3_mutex_try(). On those systems, sqlite3_mutex_try() will always return SQLITE_BUSY.
 *  The SQLite core only ever uses sqlite3_mutex_try() as an optimization so this is acceptable behavior."
 */
int cMutexTry(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexTry(mutex)\n");
    #endif

}

/* exits the given mutex.
 * behavior is undefined if the mutex wasn't entered by the calling thread
 */
void cMutexLeave(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexLeave(mutex)\n");
    #endif

}

/* returns true if this mutex is held by the calling thread
 *
 * this is used only in SQLite assert()'s, so a working
 * implementation isn't really needed; this can just return TRUE
 */
int cMutexHeld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexHeld(mutex)\n");
    #endif

    return 1;
}

/* returns true if this mutex is NOT held by the calling thread
 *
 * this is used only in SQLite assert()'s, so a working
 * implementation isn't really needed; this can just return TRUE
 */
int cMutexNotheld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexNotheld(mutex)\n");
    #endif

    return 1;
}

#endif // SQLITE_OS_OTHER