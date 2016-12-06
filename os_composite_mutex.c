#include "sqlite3.h"
#if SQLITE_OS_OTHER && SQLITE_THREADSAFE

#include "os_composite.h"

int cMutexInit() {
    return SQLITE_OK;
}

int cMutexEnd() {
    return SQLITE_OK;
}

/* creates a mutex of the given type
 * @param mutexType one of SQLITE_MUTEX_*
 * @return a pointer to a mutex, or NULL if it couldn't be created
 */
sqlite3_mutex* cMutexAlloc(int mutexType) {
    return 0;
}

void cMutexFree(sqlite3_mutex *mutex) {
}

/* tries to enter the given mutex.
 * if another thread is in the mutex, this method will block.
 */
void cMutexEnter(sqlite3_mutex *mutex) {
}

/* tries to enter the given mutex.
 * if another thread is in the mutex, this method will return SQLITE_BUSY.
 *
 * "Some systems ... do not support the operation implemented by sqlite3_mutex_try(). On those systems, sqlite3_mutex_try() will always return SQLITE_BUSY.
 *  The SQLite core only ever uses sqlite3_mutex_try() as an optimization so this is acceptable behavior."
 */
int cMutexTry(sqlite3_mutex *mutex) {
    return SQLITE_BUSY;
}

/* exits the given mutex.
 * behavior is undefined if the mutex wasn't entered by the calling thread
 */
void cMutexLeave(sqlite3_mutex *mutex) {
}

/* returns true if this mutex is held by the calling thread
 *
 * this is used only in SQLite assert()'s, so a working
 * implementation isn't really needed; this can just return TRUE
 */
int cMutexHeld(sqlite3_mutex *mutex) {
    return 1;
}

/* returns true if this mutex is NOT held by the calling thread
 *
 * this is used only in SQLite assert()'s, so a working
 * implementation isn't really needed; this can just return TRUE
 */
int cMutexNotheld(sqlite3_mutex *mutex) {
    return 1;
}

#endif // SQLITE_OS_OTHER && SQLITE_THREADSAFE