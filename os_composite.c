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

#include <stdio.h>

/* sqlite3_io_methods */
struct cFile {
    struct sqlite3_io_methods* composite_io_methods;
};

static int cClose(sqlite3_file* file) {}
static int cRead(sqlite3_file* file, void* buf, int iAmt, sqlite3_int64 iOfst) {}
static int cWrite(sqlite3_file* file, const void* buf, int iAmt, sqlite3_int64 iOfst) {}
static int cTruncate(sqlite3_file* file, sqlite3_int64 size) {}
static int cSync(sqlite3_file* file, int flags) {}
static int cFileSize(sqlite3_file* file, sqlite3_int64 *pSize) {}
static int cLock(sqlite3_file* file, int i) {}
static int cUnlock(sqlite3_file* file, int i) {}
static int cCheckReservedLock(sqlite3_file* file, int *pResOut) {}
static int cFileControl(sqlite3_file* file, int op, void *pArg) {}
static int cSectorSize(sqlite3_file* file) {}
static int cDeviceCharacteristics(sqlite3_file* file) {}
static int cShmMap(sqlite3_file* file, int iPg, int pgsz, int i, void volatile** v) {}
static int cShmLock(sqlite3_file* file, int offset, int n, int flags) {}
static void cShmBarrier(sqlite3_file* file) {}
static int cShmUnmap(sqlite3_file* file, int deleteFlag) {}
static int cFetch(sqlite3_file* file, sqlite3_int64 iOfst, int iAmt, void **pp) {}
static int cUnfetch(sqlite3_file* file, sqlite3_int64 iOfst, void *p) {}

/** sqlite3_vfs methods */
static int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
    printf("cOpen(vfs = <ptr>, zName = %s, file = <file>, flags = <flags>)\n", zName);
    struct cFile* file = (struct cFile*)baseFile;
    file->composite_io_methods = composite_io_methods;
    return SQLITE_OK;
}
static int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {}
static int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {}
static int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {}

/* attempts to return nByte bytes of randomness.
 * @return the actual number of bytes of randomness
 */
static int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {}

/* sleep for at least the given number of microseconds
 */
static int cSleep(sqlite3_vfs* vfs, int microseconds) {}

static int cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {}

/* "returns a Julian Day Number for the current date and time as a floating point value"
 */
static int cCurrentTime(sqlite3_vfs* vfs, double* time) {}

/* "returns, as an integer, the Julian Day Number multiplied by 86400000 (the number of milliseconds in a 24-hour day)"
 */
static int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {}

static const struct sqlite3_io_methods composite_io_methods = {
    .iVersion = 3,
    .xClose = cClose,
    .xRead = cRead,
    .xWrite = cWrite,
    .xTruncate = cTruncate,
    .xSync = cSync,
    .xFileSize = cFileSize,
    .xLock = cLock,
    .xUnlock = cUnlock,
    .xCheckReservedLock = cCheckReservedLock,
    .xFileControl = cFileControl,
    .xSectorSize = cSectorSize,
    .xDeviceCharacteristics = cDeviceCharacteristics,
    .xShmMap = cShmMap,
    .xShmLock = cShmLock,
    .xShmBarrier = cShmBarrier,
    .xShmUnmap = cShmUnmap,
    .xFetch = cFetch,
    .xUnfetch = cUnfetch
};

static sqlite3_vfs composite_vfs = {
    .iVersion = 3,
    .szOsFile = sizeof(struct cFile),
    .mxPathname = 255, /* what is the actual MAX_PATH? */
    .pNext = 0,
    .zName = "composite",
    .pAppData = 0,
    .xOpen = cOpen,
    .xDelete = cDelete,
    .xAccess = cAccess,
    .xFullPathname = cFullPathname,
    .xDlOpen = 0,
    .xDlError = 0,
    .xDlSym = 0,
    .xDlClose = 0,
    .xRandomness = cRandomness,
    .xSleep = cSleep,
    .xCurrentTime = cCurrentTime,
    .xGetLastError = cGetLastError,
    .xCurrentTimeInt64 = cCurrentTimeInt64,
    .xSetSystemCall = 0,
    .xGetSystemCall = 0,
    .xNextSystemCall = 0
};

/* init the OS interface */
int sqlite3_os_init(void){ 
  sqlite3_vfs_register(&composite_vfs, 1);
  return SQLITE_OK;
}

/* shutdown the OS interface */
int sqlite3_os_end(void) { 
  return SQLITE_OK; 
}

#endif
