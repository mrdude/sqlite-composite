/*
** 2004 May 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains the VFS implementation for Composite.
*/
#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_composite.h"

/* sqlite_io function prototypes */
static int _cClose(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 80, "cClose(file = '%s') => ", file->zName);
    #endif

    const int res = cClose(baseFile);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 80, "cRead(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")", file->zName, iAmt, iOfst);
    #endif

    const int res = cRead(baseFile, buf, iAmt, iOfst);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 80, "cWrite(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")", file->zName, iAmt, iOfst);
    #endif

    const int res = cWrite(baseFile, buf, iAmt, iOfst);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 80, "cTruncate(file = %s, size = %" PRIu64 ")", file->zName, size);
    #endif

    const int res = cTruncate(baseFile, size);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s =>", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cSync(sqlite3_file* baseFile, int flags) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[160];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 160, "cSync(file = %s, flags = [", file->zName);
        if( flags & SQLITE_SYNC_NORMAL ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SYNC_NORMAL ");
        if( flags & SQLITE_SYNC_FULL ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SYNC_FULL ");
        if( flags & SQLITE_SYNC_DATAONLY ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SYNC_DATAONLY ");
        snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), "])\n");
    #endif

    const int res = cSync(baseFile, flags);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 80, "cFileSize(file = %s, pSize = <>)", file->zName);
    #endif

    const int res = cFileSize(baseFile, pSize);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf(", pSize = %" PRIu64 "\n", *pSize);
    #endif

    return res;
}

static int _cLock(sqlite3_file* baseFile, int i) {
    return cLock(baseFile, i);
}

static int _cUnlock(sqlite3_file* baseFile, int i) {
    return cUnlock(baseFile, i);
}

static int _cCheckReservedLock(sqlite3_file* baseFile, int *pResOut) {
    return cCheckReservedLock(baseFile, pResOut);
}

static int _cFileControl(sqlite3_file* baseFile, int op, void *pArg) {
    return cFileControl(baseFile, op, pArg);
}

static int _cSectorSize(sqlite3_file* baseFile) {
    return cSectorSize(baseFile);
}

static int _cDeviceCharacteristics(sqlite3_file* baseFile) {
    return cDeviceCharacteristics(baseFile);
}

static int _cShmMap(sqlite3_file* baseFile, int iPg, int pgsz, int i, void volatile** v) {
    return cShmMap(baseFile, iPg, pgsz, i, v);
}

static int _cShmLock(sqlite3_file* baseFile, int offset, int n, int flags) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        struct cFile* file = (struct cFile*)baseFile;
        snprintf(ch, 160, "cShmLock(file = %s, offset = %d, n = %d, flags = [", file->zName, offset, n);

        /* these are the only valid flag combinations */
        if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_SHARED ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SHM_LOCK | SHM_SHARED ");
        if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SHM_LOCK | SHM_EXCLUSIVE ");
        if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SHM_UNLOCK | SHM_SHARED ");
        if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " SHM_UNLOCK | SHM_EXCLUSIVE ");

        snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), "])");
    #endif

    const int res = cShmLock(baseFile, offset, n, flags);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static void _cShmBarrier(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmBarrier(file = %s)\n", file->zName);
    #endif

    cShmBarrier(baseFile);
}

static int _cShmUnmap(sqlite3_file* baseFile, int deleteFlag) {
    return cShmUnmap(baseFile, deleteFlag);
}

static int _cFetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
    return cFetch(baseFile, iOfst, iAmt, pp);
}

static int _cUnfetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, void *p) {
    return cUnfetch(baseFile, iOfst, p);
}

/* sqlite_vfs function prototypes */
static int _cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[160];
        snprintf(ch, 160, "cOpen(vfs = <ptr>, zName = '%s', file = <not initialized>, flags = [", zName);
        if( flags & SQLITE_OPEN_MAIN_DB )        snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_MAIN_DB ");
        if( flags & SQLITE_OPEN_MAIN_JOURNAL )   snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_MAIN_JOURNAL ");
        if( flags & SQLITE_OPEN_TEMP_DB )        snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_TEMP_DB ");
        if( flags & SQLITE_OPEN_TEMP_JOURNAL )   snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_TEMP_JOURNAL ");
        if( flags & SQLITE_OPEN_TRANSIENT_DB )   snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_TRANSIENT_DB ");
        if( flags & SQLITE_OPEN_SUBJOURNAL )     snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_SUBJOURNAL ");
        if( flags & SQLITE_OPEN_MASTER_JOURNAL ) snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_MASTER_JOURNAL ");
        if( flags & SQLITE_OPEN_WAL )            snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_WAL ");
        //
        if( flags & SQLITE_OPEN_READWRITE )      snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_READWRITE ");
        if( flags & SQLITE_OPEN_CREATE )         snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_CREATE ");
        if( flags & SQLITE_OPEN_READONLY )       snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_READONLY ");
        //
        if( flags & SQLITE_OPEN_DELETEONCLOSE )  snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_DELETEONCLOSE ");
        if( flags & SQLITE_OPEN_EXCLUSIVE )      snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), " OPEN_EXCLUSIVE ");
        snprintf(&ch[ strlen(ch) ], 160 - strlen(ch), "])");
    #endif

    const int res = cOpen(vfs, zName, baseFile, flags, pOutFlags);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    return cDelete(vfs, zName, syncDir);
}

static int _cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        snprintf(ch, 80, "cAccess(vfs = <ptr>, zName = %s, flags = [", zName);
        if( flags == SQLITE_ACCESS_EXISTS ) snprintf(&ch[ strlen(ch) ], 80 - strlen(ch), " ACCESS_EXISTS ");
        if( flags == SQLITE_ACCESS_READWRITE ) snprintf(&ch[ strlen(ch) ], 80 - strlen(ch), " ACCESS_READWRITE ");
        if( flags == SQLITE_ACCESS_READ ) snprintf(&ch[ strlen(ch) ], 80 - strlen(ch), " ACCESS_READ ");
        snprintf(&ch[ strlen(ch) ], 80 - strlen(ch), "], pResOut = <flags>)\n");
    #endif

    const int res = cAccess(vfs, zName, flags, pResOut);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf(", pResOut = %d\n", *pResOut);
    #endif

    return res;
}

static int _cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
        char ch[80];
        snprintf(ch, 80, "cFullPathname(vfs = <ptr>, zName = %s, nOut = %d, zOut = <...>)", zName, nOut);
    #endif

    const int res = cFullPathname(vfs, zName, nOut, zOut);

    #if SQLITE_COS_PROFILE_VFS
        printf("%s => ", &ch[0]);
        PRINT_ERR_CODE(res);
        printf(", zOut = %s\n", zOut);
    #endif

    return res;
}

static int _cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    return cRandomness(vfs, nByte, zOut);
}

static int _cSleep(sqlite3_vfs* vfs, int microseconds) {
    return cSleep(vfs, microseconds);
}

static int _cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    return cGetLastError(vfs, i, ch);
}

static int _cCurrentTime(sqlite3_vfs* vfs, double* time) {
    return cCurrentTime(vfs, time);
}

static int _cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    return cCurrentTimeInt64(vfs, time);
}

/* sqlite_mutex function prototypes */
static int _cMutexInit(void) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexInit()");
    #endif

    const int res = cMutexInit();

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => ");
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static int _cMutexEnd(void) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexEnd()");
    #endif

    const int res = cMutexEnd();

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => ");
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static sqlite3_mutex* _cMutexAlloc(int mutexType) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(160);
        CTRACE_APPEND("cMutexAlloc(mutexType = ");
        switch( mutexType ) {
            case SQLITE_MUTEX_FAST: CTRACE_APPEND(" MUTEX_FAST "); break;
            case SQLITE_MUTEX_RECURSIVE: CTRACE_APPEND(" MUTEX_RECURSIVE "); break;
            case SQLITE_MUTEX_STATIC_MASTER: CTRACE_APPEND(" MUTEX_STATIC_MASTER "); break;
            case SQLITE_MUTEX_STATIC_MEM: CTRACE_APPEND(" MUTEX_STATIC_MEM "); break;
            case SQLITE_MUTEX_STATIC_OPEN: CTRACE_APPEND(" MUTEX_STATIC_OPEN "); break;
            case SQLITE_MUTEX_STATIC_PRNG: CTRACE_APPEND(" MUTEX_STATIC_PRNG "); break;
            case SQLITE_MUTEX_STATIC_LRU: CTRACE_APPEND(" MUTEX_STATIC_LRU "); break;
            case SQLITE_MUTEX_STATIC_PMEM: CTRACE_APPEND(" MUTEX_STATIC_PMEM "); break;
            case SQLITE_MUTEX_STATIC_APP1: CTRACE_APPEND(" MUTEX_STATIC_APP1 "); break;
            case SQLITE_MUTEX_STATIC_APP2: CTRACE_APPEND(" MUTEX_STATIC_APP2 "); break;
            case SQLITE_MUTEX_STATIC_APP3: CTRACE_APPEND(" MUTEX_STATIC_APP3 "); break;
            case SQLITE_MUTEX_STATIC_VFS1: CTRACE_APPEND(" MUTEX_STATIC_VFS1 "); break;
            case SQLITE_MUTEX_STATIC_VFS2: CTRACE_APPEND(" MUTEX_STATIC_VFS2 "); break;
            case SQLITE_MUTEX_STATIC_VFS3: CTRACE_APPEND(" MUTEX_STATIC_VFS3 "); break; 
        }
        CTRACE_APPEND(")");
    #endif

    sqlite3_mutex* mut = cMutexAlloc(mutexType);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => mutex = %p\n", mutex);
    #endif

    return mut;
}

static void _cMutexFree(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        printf("cMutexFree(mutex)\n");
    #endif

    cMutexFree(mutex);
}

static void _cMutexEnter(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexEnter(mutex)\n");
    #endif

    cMutexEnter(mutex);
}

static int _cMutexTry(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexTry(mutex)");
    #endif

    const int res = cMutexTry(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => ");
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static void _cMutexLeave(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        printf("cMutexLeave(mutex)\n");
    #endif

    cMutexLeave(mutex);
}

static int _cMutexHeld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexHeld(mutex)");
    #endif

    const int res = cMutexHeld(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => %s\n", res ? "TRUE" : "FALSE");
    #endif

    return res;
}

static int _cMutexNotheld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexNotheld(mutex)");
    #endif

    return cMutexNotheld(mutex);

    const int res = cMutexNotheld(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
        printf(" => %s\n", res ? "TRUE" : "FALSE");
    #endif

    return res;
}

/* sqlite_mem function prototypes */
static void* _cMemMalloc(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemMalloc(sz = %d)", sz);
    #endif

    void* mem = cMemMalloc(sz);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        printf("mem = %p\n", mem);
    #endif

    return mem;
}

static void _cMemFree(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemFree(mem = %p)", mem);
    #endif

    cMemFree(mem);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
    #endif
}

static void* _cMemRealloc(void* mem, int newSize) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemRealloc(mem = %p, newSize = %d)", mem, newSize);
    #endif

    void* newPtr = cMemRealloc(mem, newSize);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        printf("newPtr = %p\n", newPtr);
    #endif

    return newPtr;
}

static int _cMemSize(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemSize(mem = %p)", mem);
    #endif

    const int sz = cMemSize(mem);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        printf("sz = %d\n", sz);
    #endif

    return sz;
}

static int _cMemRoundup(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemRoundup(sz = %d)", sz);
    #endif

    const int newSz = cMemRoundup(sz);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        printf("newSz = %d\n", newSz);
    #endif

    return sz;
}

static int _cMemInit(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemInit(pAppData = <>)");
    #endif

    const int res = cMemInit(pAppData);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        PRINT_ERR_CODE(res);
        printf("\n");
    #endif

    return res;
}

static void _cMemShutdown(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMemShutdown(pAppData = <>)");
    #endif

    cMemShutdown(pAppData);

    #if SQLITE_COS_PROFILE_MEMORY
        CTRACE_PRINT();
        printf("\n");
    #endif
}

/* API structs */
struct sqlite3_io_methods composite_io_methods = {
    .iVersion = 3,
    .xClose = _cClose,
    .xRead = _cRead,
    .xWrite = _cWrite,
    .xTruncate = _cTruncate,
    .xSync = _cSync,
    .xFileSize = _cFileSize,
    .xLock = _cLock,
    .xUnlock = _cUnlock,
    .xCheckReservedLock = _cCheckReservedLock,
    .xFileControl = _cFileControl,
    .xSectorSize = _cSectorSize,
    .xDeviceCharacteristics = _cDeviceCharacteristics,
    .xShmMap = _cShmMap,
    .xShmLock = _cShmLock,
    .xShmBarrier = _cShmBarrier,
    .xShmUnmap = _cShmUnmap,
    .xFetch = _cFetch,
    .xUnfetch = _cUnfetch
};

struct composite_vfs_data composite_vfs_app_data;

static sqlite3_vfs composite_vfs = {
    .iVersion = 2,
    .szOsFile = sizeof(struct cFile),
    .mxPathname = 255, /* what is the actual MAX_PATH? */
    .pNext = 0,
    .zName = "composite",
    .pAppData = &composite_vfs_app_data,
    .xOpen = _cOpen,
    .xDelete = _cDelete,
    .xAccess = _cAccess,
    .xFullPathname = _cFullPathname,
    .xDlOpen = 0,
    .xDlError = 0,
    .xDlSym = 0,
    .xDlClose = 0,
    .xRandomness = _cRandomness,
    .xSleep = _cSleep,
    .xCurrentTime = _cCurrentTime,
    .xGetLastError = _cGetLastError,
    .xCurrentTimeInt64 = _cCurrentTimeInt64,
    /* everything above is required in versions 1-2 */
    .xSetSystemCall = 0,
    .xGetSystemCall = 0,
    .xNextSystemCall = 0
};

static const sqlite3_mutex_methods composite_mutex_methods = {
    .xMutexInit = _cMutexInit,
    .xMutexEnd = _cMutexEnd,
    .xMutexAlloc = _cMutexAlloc,
    .xMutexFree = _cMutexFree,
    .xMutexEnter = _cMutexEnter,
    .xMutexTry = _cMutexTry,
    .xMutexLeave = _cMutexLeave,
    .xMutexHeld = _cMutexHeld,
    .xMutexNotheld = _cMutexNotheld
};

static const sqlite3_mem_methods composite_mem_methods = {
    .xMalloc = _cMemMalloc,
    .xFree = _cMemFree,
    .xRealloc = _cMemRealloc,
    .xSize = _cMemSize,
    .xRoundup = _cMemRoundup,
    .xInit = _cMemInit,
    .xShutdown = _cMemShutdown,
    .pAppData = 0
};

/* init the OS interface */
int sqlite3_os_init(void){
  #if SQLITE_THREADSAFE >= 1
    //sqlite3_config(SQLITE_CONFIG_MUTEX, &composite_mutex_methods);
  #endif
  //sqlite3_config(SQLITE_CONFIG_MALLOC, &composite_mem_methods);
  sqlite3_vfs_register(&composite_vfs, 1);

  struct composite_vfs_data *data = &composite_vfs_app_data;
  data->random_fd = fopen("/dev/random", "rb");

  return SQLITE_OK;
}

/* shutdown the OS interface */
int sqlite3_os_end(void) {
  struct composite_vfs_data *data = &composite_vfs_app_data;
  fclose(data->random_fd);
  return SQLITE_OK; 
}

#endif
