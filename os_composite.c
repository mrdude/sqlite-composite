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

/* API structs */
struct sqlite3_io_methods composite_io_methods = {
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

struct composite_vfs_data composite_vfs_app_data;

static sqlite3_vfs composite_vfs = {
    .iVersion = 3,
    .szOsFile = sizeof(struct cFile),
    .mxPathname = 255, /* what is the actual MAX_PATH? */
    .pNext = 0,
    .zName = "composite",
    .pAppData = &composite_vfs_app_data,
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

static const sqlite3_mutex_methods composite_mutex_methods = {
    .xMutexInit = cMutexInit,
    .xMutexEnd = cMutexEnd,
    .xMutexAlloc = cMutexAlloc,
    .xMutexFree = cMutexFree,
    .xMutexEnter = cMutexEnter,
    .xMutexTry = cMutexTry,
    .xMutexLeave = cMutexLeave,
    .xMutexHeld = cMutexHeld,
    .xMutexNotheld = cMutexNotheld
};

static const sqlite3_mem_methods composite_mem_methods = {
    .xMalloc = cMemMalloc,
    .xFree = cMemFree,
    .xRealloc = cMemRealloc,
    .xSize = cMemSize,
    .xRoundup = cMemRoundup,
    .xInit = cMemInit,
    .xShutdown = cMemShutdown,
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
