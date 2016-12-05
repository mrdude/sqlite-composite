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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "os_composite.h"

/* sqlite_io function prototypes */
static int _cClose(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cClose(file = '%s')", file->zName);
    #endif

    const int res = cClose(baseFile);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cRead(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")", file->zName, iAmt, iOfst);
    #endif

    const int res = cRead(baseFile, buf, iAmt, iOfst);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cWrite(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")", file->zName, iAmt, iOfst);
    #endif

    const int res = cWrite(baseFile, buf, iAmt, iOfst);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cTruncate(file = %s, size = %" PRIu64 ")", file->zName, size);
    #endif

    const int res = cTruncate(baseFile, size);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cSync(sqlite3_file* baseFile, int flags) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(160);
        CTRACE_APPEND("cSync(file = %s, flags = [", file->zName);
        if( flags & SQLITE_SYNC_NORMAL ) CTRACE_APPEND(" SYNC_NORMAL ");
        if( flags & SQLITE_SYNC_FULL ) CTRACE_APPEND(" SYNC_FULL ");
        if( flags & SQLITE_SYNC_DATAONLY ) CTRACE_APPEND(" SYNC_DATAONLY ");
        CTRACE_APPEND("])");
    #endif

    const int res = cSync(baseFile, flags);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cFileSize(file = %s, pSize = <>)", file->zName);
    #endif

    const int res = cFileSize(baseFile, pSize);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", pSize = %" PRIu64, *pSize);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cLock(sqlite3_file* baseFile, int lockType) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cLock(file = %s, lockType = ", file->zName);
        if( lockType == SQLITE_LOCK_NONE )      CTRACE_APPEND("LOCK_NONE");
        if( lockType == SQLITE_LOCK_SHARED )    CTRACE_APPEND("LOCK_SHARED");
        if( lockType == SQLITE_LOCK_RESERVED )  CTRACE_APPEND("LOCK_RESERVED");
        if( lockType == SQLITE_LOCK_PENDING )   CTRACE_APPEND("LOCK_PENDING");
        if( lockType == SQLITE_LOCK_EXCLUSIVE ) CTRACE_APPEND("LOCK_EXCLUSIVE");
        CTRACE_APPEND(")");
    #endif

    const int res = cLock(baseFile, lockType);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cUnlock(sqlite3_file* baseFile, int lockType) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cUnlock(file = %s, lockType = ", file->zName);
        if( lockType == SQLITE_LOCK_NONE )      CTRACE_APPEND("LOCK_NONE");
        if( lockType == SQLITE_LOCK_SHARED )    CTRACE_APPEND("LOCK_SHARED");
        if( lockType == SQLITE_LOCK_RESERVED )  CTRACE_APPEND("LOCK_RESERVED");
        if( lockType == SQLITE_LOCK_PENDING )   CTRACE_APPEND("LOCK_PENDING");
        if( lockType == SQLITE_LOCK_EXCLUSIVE ) CTRACE_APPEND("LOCK_EXCLUSIVE");
        CTRACE_APPEND(")");
    #endif

    const int res = cUnlock(baseFile, lockType);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cCheckReservedLock(sqlite3_file* baseFile, int *pResOut) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cCheckReservedLock(file = %s)", file->zName);
    #endif

    const int res = cCheckReservedLock(baseFile, pResOut);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", pResOut = %d", pResOut ? *pResOut : -1);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cFileControl(sqlite3_file* baseFile, int op, void *pArg) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cFileControl(file = %s, op = %d, pArg = <...>)", file->zName, op);
    #endif

    const int res = cFileControl(baseFile, op, pArg);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cSectorSize(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cSectorSize(file = %s)", file->zName);
    #endif

    const int sectorSize = cSectorSize(baseFile);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => %d", sectorSize);
        CTRACE_PRINT();
    #endif

    return sectorSize;
}

static int _cDeviceCharacteristics(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(256);
        CTRACE_APPEND("cDeviceCharacteristics(file = %s)", file->zName);
    #endif

    const int flags = cDeviceCharacteristics(baseFile);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => flags = [");
        if( flags & SQLITE_IOCAP_ATOMIC ) CTRACE_APPEND(" IOCAP_ATOMIC ");
        if( flags & SQLITE_IOCAP_ATOMIC512 ) CTRACE_APPEND(" IOCAP_ATOMIC512 ");
        if( flags & SQLITE_IOCAP_ATOMIC1K ) CTRACE_APPEND(" IOCAP_ATOMIC1K ");
        if( flags & SQLITE_IOCAP_ATOMIC2K ) CTRACE_APPEND(" IOCAP_ATOMIC2K ");
        if( flags & SQLITE_IOCAP_ATOMIC4K ) CTRACE_APPEND(" IOCAP_ATOMIC4K ");
        if( flags & SQLITE_IOCAP_ATOMIC8K ) CTRACE_APPEND(" IOCAP_ATOMIC8K ");
        if( flags & SQLITE_IOCAP_ATOMIC16K ) CTRACE_APPEND(" IOCAP_ATOMIC16K ");
        if( flags & SQLITE_IOCAP_ATOMIC32K ) CTRACE_APPEND(" IOCAP_ATOMIC32K ");
        if( flags & SQLITE_IOCAP_ATOMIC64K ) CTRACE_APPEND(" IOCAP_ATOMIC64K ");
        if( flags & SQLITE_IOCAP_SAFE_APPEND ) CTRACE_APPEND(" IOCAP_SAFE_APPEND ");
        if( flags & SQLITE_IOCAP_SEQUENTIAL ) CTRACE_APPEND(" IOCAP_SEQUENTIAL ");
        CTRACE_APPEND("]");
        CTRACE_PRINT();
    #endif

    return flags;
}

static int _cShmMap(sqlite3_file* baseFile, int iPg, int pgsz, int i, void volatile** v) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cShmMap(file = %s, iPg = %d, pgsz = %d, i = %d, v = <>)", file->zName, iPg, pgsz, i);
    #endif

    const int res = cShmMap(baseFile, iPg, pgsz, i, v);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cShmLock(sqlite3_file* baseFile, int offset, int n, int flags) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(160);
        CTRACE_APPEND("cShmLock(file = %s, offset = %d, n = %d, flags = [", file->zName, offset, n);

        /* these are the only valid flag combinations */
        if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_SHARED ) CTRACE_APPEND(" SHM_LOCK | SHM_SHARED ");
        if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE ) CTRACE_APPEND(" SHM_LOCK | SHM_EXCLUSIVE ");
        if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED ) CTRACE_APPEND(" SHM_UNLOCK | SHM_SHARED ");
        if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE ) CTRACE_APPEND(" SHM_UNLOCK | SHM_EXCLUSIVE ");

        CTRACE_APPEND("])");
    #endif

    const int res = cShmLock(baseFile, offset, n, flags);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static void _cShmBarrier(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cShmBarrier(file = %s)", file->zName);
    #endif

    cShmBarrier(baseFile);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_PRINT();
    #endif
}

static int _cShmUnmap(sqlite3_file* baseFile, int deleteFlag) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cShmUnmap(file = %s, deleteFlag = %d)", file->zName, deleteFlag);
    #endif

    const int res = cShmUnmap(baseFile, deleteFlag);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cFetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cFetch(file = %s, iOfst = %" PRIu64 ", iAmt = %d, pp = <>)", file->zName, iOfst, iAmt);
    #endif

    const int res = cFetch(baseFile, iOfst, iAmt, pp);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cUnfetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, void *p) {
    #if SQLITE_COS_PROFILE_VFS
        struct cFile* file = (struct cFile*)baseFile;
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cUnfetch(file = %s, iOfst = %" PRIu64 ", pp = <>)", file->zName, iOfst);
    #endif

    const int res = cUnfetch(baseFile, iOfst, p);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

/* sqlite_vfs function prototypes */
static int _cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(320);
        CTRACE_APPEND("cOpen(vfs = <ptr>, zName = '%s', file = <not initialized>, flags = [", zName);
        if( flags & SQLITE_OPEN_MAIN_DB )        CTRACE_APPEND(" OPEN_MAIN_DB ");
        if( flags & SQLITE_OPEN_MAIN_JOURNAL )   CTRACE_APPEND(" OPEN_MAIN_JOURNAL ");
        if( flags & SQLITE_OPEN_TEMP_DB )        CTRACE_APPEND(" OPEN_TEMP_DB ");
        if( flags & SQLITE_OPEN_TEMP_JOURNAL )   CTRACE_APPEND(" OPEN_TEMP_JOURNAL ");
        if( flags & SQLITE_OPEN_TRANSIENT_DB )   CTRACE_APPEND(" OPEN_TRANSIENT_DB ");
        if( flags & SQLITE_OPEN_SUBJOURNAL )     CTRACE_APPEND(" OPEN_SUBJOURNAL ");
        if( flags & SQLITE_OPEN_MASTER_JOURNAL ) CTRACE_APPEND(" OPEN_MASTER_JOURNAL ");
        if( flags & SQLITE_OPEN_WAL )            CTRACE_APPEND(" OPEN_WAL ");
        //
        if( flags & SQLITE_OPEN_READWRITE )      CTRACE_APPEND(" OPEN_READWRITE ");
        if( flags & SQLITE_OPEN_CREATE )         CTRACE_APPEND(" OPEN_CREATE ");
        if( flags & SQLITE_OPEN_READONLY )       CTRACE_APPEND(" OPEN_READONLY ");
        //
        if( flags & SQLITE_OPEN_DELETEONCLOSE )  CTRACE_APPEND(" OPEN_DELETEONCLOSE ");
        if( flags & SQLITE_OPEN_EXCLUSIVE )      CTRACE_APPEND(" OPEN_EXCLUSIVE ");
        CTRACE_APPEND("], pOutFlags = <...>)");
    #endif

    const int res = cOpen(vfs, zName, baseFile, flags, pOutFlags);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", pOutFlags = [");
        if( *pOutFlags & SQLITE_OPEN_MAIN_DB )        CTRACE_APPEND(" OPEN_MAIN_DB ");
        if( *pOutFlags & SQLITE_OPEN_MAIN_JOURNAL )   CTRACE_APPEND(" OPEN_MAIN_JOURNAL ");
        if( *pOutFlags & SQLITE_OPEN_TEMP_DB )        CTRACE_APPEND(" OPEN_TEMP_DB ");
        if( *pOutFlags & SQLITE_OPEN_TEMP_JOURNAL )   CTRACE_APPEND(" OPEN_TEMP_JOURNAL ");
        if( *pOutFlags & SQLITE_OPEN_TRANSIENT_DB )   CTRACE_APPEND(" OPEN_TRANSIENT_DB ");
        if( *pOutFlags & SQLITE_OPEN_SUBJOURNAL )     CTRACE_APPEND(" OPEN_SUBJOURNAL ");
        if( *pOutFlags & SQLITE_OPEN_MASTER_JOURNAL ) CTRACE_APPEND(" OPEN_MASTER_JOURNAL ");
        if( *pOutFlags & SQLITE_OPEN_WAL )            CTRACE_APPEND(" OPEN_WAL ");
        //
        if( *pOutFlags & SQLITE_OPEN_READWRITE )      CTRACE_APPEND(" OPEN_READWRITE ");
        if( *pOutFlags & SQLITE_OPEN_CREATE )         CTRACE_APPEND(" OPEN_CREATE ");
        if( *pOutFlags & SQLITE_OPEN_READONLY )       CTRACE_APPEND(" OPEN_READONLY ");
        //
        if( *pOutFlags & SQLITE_OPEN_DELETEONCLOSE )  CTRACE_APPEND(" OPEN_DELETEONCLOSE ");
        if( *pOutFlags & SQLITE_OPEN_EXCLUSIVE )      CTRACE_APPEND(" OPEN_EXCLUSIVE ");
        CTRACE_APPEND("]");
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(160);
        CTRACE_APPEND("cDelete(vfs = <ptr>, zName = %s, syncDir = %d)", zName, syncDir);
    #endif

    const int res = cDelete(vfs, zName, syncDir);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(160);
        CTRACE_APPEND("cAccess(vfs = <ptr>, zName = %s, flags = [", zName);
        if( flags == SQLITE_ACCESS_EXISTS ) CTRACE_APPEND(" ACCESS_EXISTS ");
        if( flags == SQLITE_ACCESS_READWRITE ) CTRACE_APPEND(" ACCESS_READWRITE ");
        if( flags == SQLITE_ACCESS_READ ) CTRACE_APPEND(" ACCESS_READ ");
        CTRACE_APPEND("], pResOut = <flags>)");
    #endif

    const int res = cAccess(vfs, zName, flags, pResOut);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", pResOut = %d", *pResOut);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cFullPathname(vfs = <ptr>, zName = %s, nOut = %d, zOut = <...>)", zName, nOut);
    #endif

    const int res = cFullPathname(vfs, zName, nOut, zOut);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", zOut = %s\n", zOut);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cRandomness(vfs = <ptr>, nByte = %d, zOut = <ptr>)", nByte);
    #endif

    const int bytesOfRandomness = cRandomness(vfs, nByte, zOut);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        CTRACE_APPEND("bytesOfRandom = %d, zOut = <...>\n", bytesOfRandomness);
        CTRACE_PRINT();
    #endif

    return bytesOfRandomness;
}

static int _cSleep(sqlite3_vfs* vfs, int microseconds) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cSleep(vfs = <vfs>, microseconds = %d)\n", microseconds);
    #endif

    const int res = cSleep(vfs, microseconds);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cGetLastError(vfs = <vfs>, i = %d, ch = %s)", i, ch);
    #endif

    const int res = cGetLastError(vfs, i, ch);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cCurrentTime(sqlite3_vfs* vfs, double* time) {
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cCurrentTime(vfs = <vfs>, time = <...>)");
    #endif

    const int res = cCurrentTime(vfs, time);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", time = %f", *time);
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    return cCurrentTimeInt64(vfs, time);
    #if SQLITE_COS_PROFILE_VFS
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cCurrentTimeInt64(vfs = <vfs>, time = <...>)");
    #endif

    const int res = cCurrentTimeInt64(vfs, time);

    #if SQLITE_COS_PROFILE_VFS
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_APPEND(", time = %" PRIu64 "\n", *time);
        CTRACE_PRINT();
    #endif

    return res;
}

/* sqlite_mutex function prototypes */
static int _cMutexInit(void) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexInit()");
    #endif

    const int res = cMutexInit();

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
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
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
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
        CTRACE_APPEND(" => mutex = %p", mutex);
        CTRACE_PRINT();
    #endif

    return mut;
}

static void _cMutexFree(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexFree(mutex = %p)", mutex);
    #endif

    cMutexFree(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
    #endif
}

static void _cMutexEnter(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexEnter(mutex = %p)", mutex);
    #endif

    cMutexEnter(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
    #endif
}

static int _cMutexTry(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexTry(mutex)");
    #endif

    const int res = cMutexTry(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
    #endif

    return res;
}

static void _cMutexLeave(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexLeave(mutex = %p)", mutex);
    #endif

    cMutexLeave(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_PRINT();
    #endif
}

static int _cMutexHeld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexHeld(mutex = %p)", mutex);
    #endif

    const int res = cMutexHeld(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_APPEND(" => %s", res ? "TRUE" : "FALSE");
        CTRACE_PRINT();
    #endif

    return res;
}

static int _cMutexNotheld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_STRING_DEF(80);
        CTRACE_APPEND("cMutexNotheld(mutex = %p)", mutex);
    #endif

    const int res = cMutexNotheld(mutex);

    #if SQLITE_COS_PROFILE_MUTEX
        CTRACE_APPEND(" => %s", res ? "TRUE" : "FALSE");
        CTRACE_PRINT();
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
        CTRACE_APPEND(" => mem = %p", mem);
        CTRACE_PRINT();
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
        CTRACE_PRINT(" => newPtr = %p\n", newPtr);
        CTRACE_PRINT();
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
        CTRACE_APPEND("=> sz = %d\n", sz);
        CTRACE_PRINT();
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
        CTRACE_APPEND("newSz = %d", newSz);
        CTRACE_PRINT();
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
        CTRACE_APPEND(" => ");
        APPEND_ERR_CODE(res);
        CTRACE_PRINT();
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
    .mxPathname = MAX_PATHNAME,
    .pNext = 0,
    .zName = "composite-inmemfs",
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

const sqlite3_mem_methods composite_mem_methods = {
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
  #if SQLITE_THREADSAFE
    sqlite3_config(SQLITE_CONFIG_MUTEX, &composite_mutex_methods);
  #endif
  sqlite3_config(SQLITE_CONFIG_MALLOC, &composite_mem_methods);

  struct composite_vfs_data *data = &composite_vfs_app_data;
  data->random_fd = open("/dev/random", O_RDONLY);
  sqlite3_vfs_register(&composite_vfs, 1);

  return SQLITE_OK;
}

/* shutdown the OS interface */
int sqlite3_os_end(void) {
  struct composite_vfs_data *data = &composite_vfs_app_data;
  close(data->random_fd);
  return SQLITE_OK; 
}

#endif
