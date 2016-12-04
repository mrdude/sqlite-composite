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
    return cRead(baseFile, buf, iAmt, iOfst);
}

static int _cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    return cWrite(baseFile, buf, iAmt, iOfst);
}

static int _cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    return cTruncate(baseFile, size);
}

static int _cSync(sqlite3_file* baseFile, int flags) {
    return cSync(baseFile, flags);
}

static int _cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    return cFileSize(baseFile, pSize);
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
    return cShmLock(baseFile, offset, n, flags);
}

static void _cShmBarrier(sqlite3_file* baseFile) {
    return cShmBarrier(baseFile);
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
    return cOpen(vfs, zName, baseFie, flags, pOutFlags);
}

static int _cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    return cDelete(vfs, zName, syncDir);
}

static int _cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    return cAccess(vfs, zName, flags, pResOut);
}

static int _cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    return cFullPathname(vfs, zName, nOut, zOut);
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
    return cMutexInit();
}

static int _cMutexEnd(void) {
    return cMutexEnd();
}

static sqlite3_mutex* _cMutexAlloc(int mutexType) {
    return cMutexAlloc(mutexType);
}

static void _cMutexFree(sqlite3_mutex *mutex) {
    return cMutexFree(mutex);
}

static void _cMutexEnter(sqlite3_mutex *mutex) {
    return cMutexEnter(mutex);
}

static int _cMutexTry(sqlite3_mutex *mutex) {
    return cMutexTry(mutex);
}

static void _cMutexLeave(sqlite3_mutex *mutex) {
    cMutexLeave(mutex);
}

static int _cMutexHeld(sqlite3_mutex *mutex) {
    return cMutexHeld(mutex);
}

static int _cMutexNotheld(sqlite3_mutex *mutex) {
    return cMutexNotheld(mutex);
}

/* sqlite_mem function prototypes */
static void* _cMemMalloc(int sz) {
    return cMemMalloc(sz);
}

static void _cMemFree(void* mem) {
    cMemFree(mem);
}

static void* _cMemRealloc(void* mem, int newSize) {
    return cMemRelloc(mem, newSize);
}

static int _cMemSize(void* mem) {
    return cMemSize(mem);
}

static int _cMemRoundup(int sz) {
    return cMemRoundup(sz);
}

static int _cMemInit(void* pAppData) {
    return cMemInit(pAppData);
}

static void _cMemShutdown(void* pAppData) {
    cMemShutdown(pAppData);
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
    .iVersion = 3,
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
    sqlite3_config(SQLITE_CONFIG_MUTEX, &composite_mutex_methods);
  #endif
  sqlite3_config(SQLITE_CONFIG_MALLOC, &composite_mem_methods);
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
