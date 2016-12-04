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

#ifndef SQLITE_COS_PROFILE_VFS
#define SQLITE_COS_PROFILE_VFS 0
#endif

#ifndef SQLITE_COS_PROFILE_MUTEX
#define SQLITE_COS_PROFILE_MUTEX 0
#endif

#ifndef SQLITE_COS_PROFILE_MEMORY
#define SQLITE_COS_PROFILE_MEMORY 0
#endif

/* cFile */
struct cFile {
    struct sqlite3_io_methods* composite_io_methods;
    const char* zName;
    FILE* fd;
    int closed;
    int deleteOnClose;
};

struct composite_vfs_data {
    FILE* random_fd;
};

/* sqlite_io function prototypes */
static int cClose(sqlite3_file* file);
static int cRead(sqlite3_file* file, void* buf, int iAmt, sqlite3_int64 iOfst);
static int cWrite(sqlite3_file* file, const void* buf, int iAmt, sqlite3_int64 iOfst);
static int cTruncate(sqlite3_file* file, sqlite3_int64 size);
static int cSync(sqlite3_file* file, int flags);
static int cFileSize(sqlite3_file* file, sqlite3_int64 *pSize);
static int cLock(sqlite3_file* file, int i);
static int cUnlock(sqlite3_file* file, int i);
static int cCheckReservedLock(sqlite3_file* file, int *pResOut);
static int cFileControl(sqlite3_file* file, int op, void *pArg);
static int cSectorSize(sqlite3_file* file);
static int cDeviceCharacteristics(sqlite3_file* file);
static int cShmMap(sqlite3_file* file, int iPg, int pgsz, int i, void volatile** v);
static int cShmLock(sqlite3_file* file, int offset, int n, int flags);
static void cShmBarrier(sqlite3_file* file);
static int cShmUnmap(sqlite3_file* file, int deleteFlag);
static int cFetch(sqlite3_file* file, sqlite3_int64 iOfst, int iAmt, void **pp);
static int cUnfetch(sqlite3_file* file, sqlite3_int64 iOfst, void *p);

/* sqlite_vfs function prototypes */
static int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags);
static int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir);
static int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut);
static int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut);
static int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut);
static int cSleep(sqlite3_vfs* vfs, int microseconds);
static int cGetLastError(sqlite3_vfs* vfs, int i, char *ch);
static int cCurrentTime(sqlite3_vfs* vfs, double* time);
static int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time);

/* sqlite_mutex function prototypes */
static int cMutexInit(void);
static int cMutexEnd(void);
static sqlite3_mutex* cMutexAlloc(int mutexType);
static void cMutexFree(sqlite3_mutex *mutex);
static void cMutexEnter(sqlite3_mutex *mutex);
static int cMutexTry(sqlite3_mutex *mutex);
static void cMutexLeave(sqlite3_mutex *mutex);
static int cMutexHeld(sqlite3_mutex *mutex);
static int cMutexNotheld(sqlite3_mutex *mutex);

/* sqlite_mem function prototypes */
static void *cMemMalloc(int sz);         /* Memory allocation function */
static void cMemFree(void* mem);          /* Free a prior allocation */
static void *cMemRealloc(void* mem, int newSize);  /* Resize an allocation */
static int cMemSize(void* mem);           /* Return the size of an allocation */
static int cMemRoundup(int sz);          /* Round up request size to allocation size */
static int cMemInit(void* pAppData);           /* Initialize the memory allocator */
static void cMemShutdown(void* pAppData);      /* Deinitialize the memory allocator */

/* API structs */
static struct sqlite3_io_methods composite_io_methods = {
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

static struct composite_vfs_data composite_vfs_app_data;

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

/* sqlite3_io_methods */
static int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cClose(file = '%s')\n", file->zName);
    #endif

    fclose(file->fd);
    file->closed = 1;

    //TODO delete?
}

static int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cRead(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")\n", file->zName, iAmt, iOfst);
    #endif

    /* seek to the correct position */
    if( fseek(file->fd, iOfst, SEEK_SET) != 0 ) {
        return SQLITE_IOERR;
    }

    /* read the bytes */
    int bytesRead = fread(buf, 1, iAmt, file->fd);
    if( bytesRead == iAmt ) {
        return SQLITE_OK;
    }

    if( feof(file->fd) || ferror(file->fd) ) {
        return SQLITE_IOERR;
    }

    /* if we do a short read, we have to fill the rest of the buffer with 0's */
    int i;
    for( i = bytesRead; i < iAmt; i++ )
        ((char*)buf)[i] = 0;

    return SQLITE_IOERR_SHORT_READ;
}

static int cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cWrite(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")\n", file->zName, iAmt, iOfst);
    #endif

    return SQLITE_IOERR;
}

static int cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    struct cFile* file = (struct cFile*)baseFile;
    
    #if SQLITE_COS_PROFILE_VFS
    printf("cTruncate(file = %s, size = %" PRIu64 ")\n", file->zName, size);
    #endif

    return SQLITE_IOERR;
}

static int cSync(sqlite3_file* baseFile, int flags) {
    struct cFile* file = (struct cFile*)baseFile;
    
    #if SQLITE_COS_PROFILE_VFS
    printf("cSync(file = %s, flags = [", file->zName);
    if( flags & SQLITE_SYNC_NORMAL ) printf(" SYNC_NORMAL ");
    if( flags & SQLITE_SYNC_FULL ) printf(" SYNC_FULL ");
    if( flags & SQLITE_SYNC_DATAONLY ) printf(" SYNC_DATAONLY ");
    printf("])\n");
    #endif

    return SQLITE_IOERR;
}

static int cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cFileSize(file = %s)\n", file->zName);
    #endif

    return SQLITE_IOERR;
}

/* increases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
static int cLock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cLock(file = %s, lockType = ", file->zName);
    if( lockType == SQLITE_LOCK_NONE )      printf("LOCK_NONE");
    if( lockType == SQLITE_LOCK_SHARED )    printf("LOCK_SHARED");
    if( lockType == SQLITE_LOCK_RESERVED )  printf("LOCK_RESERVED");
    if( lockType == SQLITE_LOCK_PENDING )   printf("LOCK_PENDING");
    if( lockType == SQLITE_LOCK_EXCLUSIVE ) printf("LOCK_EXCLUSIVE");
    printf(")\n");
    #endif

    return SQLITE_OK;
}

/* decreases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
static int cUnlock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cUnlock(file = %s, lockType = ", file->zName);
    if( lockType == SQLITE_LOCK_NONE )      printf("LOCK_NONE");
    if( lockType == SQLITE_LOCK_SHARED )    printf("LOCK_SHARED");
    if( lockType == SQLITE_LOCK_RESERVED )  printf("LOCK_RESERVED");
    if( lockType == SQLITE_LOCK_PENDING )   printf("LOCK_PENDING");
    if( lockType == SQLITE_LOCK_EXCLUSIVE ) printf("LOCK_EXCLUSIVE");
    printf(")\n");
    #endif

    return SQLITE_OK;
}

/* returns true if any connection has a RESERVED, PENDING, or EXCLUSIVE lock on this file
 */
static int cCheckReservedLock(sqlite3_file* baseFile, int *pResOut) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cCheckReservedLock(file = %s, lockType = ", file->zName);
    if( lockType == SQLITE_LOCK_NONE )      printf("LOCK_NONE");
    if( lockType == SQLITE_LOCK_SHARED )    printf("LOCK_SHARED");
    if( lockType == SQLITE_LOCK_RESERVED )  printf("LOCK_RESERVED");
    if( lockType == SQLITE_LOCK_PENDING )   printf("LOCK_PENDING");
    if( lockType == SQLITE_LOCK_EXCLUSIVE ) printf("LOCK_EXCLUSIVE");
    printf(")\n");
    #endif

    *pResOut = 0;

    return SQLITE_OK;
}

/* "VFS implementations should return [SQLITE_NOTFOUND] for file control opcodes that they do not recognize."
 */
static int cFileControl(sqlite3_file* baseFile, int op, void *pArg) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cFileControl(file = %s, op = %d, pArg = <>)\n", file->zName, op);
    #endif

    return SQLITE_NOTFOUND;
}

/* "The xSectorSize() method returns the sector size of the device that underlies the file."
 */
static int cSectorSize(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cSectorSize(file = %s)\n", file->zName, op);
    #endif

    return 512; //TODO
}

/* "The xDeviceCharacteristics() method returns a bit vector describing behaviors of the underlying device"
 */
static int cDeviceCharacteristics(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cDeviceCharacteristics(file = %s)\n", file->zName, op);
    #endif

    int flags = 0;
    flags |= SQLITE_IOCAP_ATOMIC; /* "The SQLITE_IOCAP_ATOMIC property means that all writes of any size are atomic." */
    //flags |= SQLITE_IOCAP_ATOMIC512; /* "The SQLITE_IOCAP_ATOMICnnn values mean that writes of blocks that are nnn bytes in size and are aligned to an address which is an integer multiple of nnn are atomic." */
    //flags |= SQLITE_IOCAP_ATOMIC1K;
    //flags |= SQLITE_IOCAP_ATOMIC2K;
    //flags |= SQLITE_IOCAP_ATOMIC4K;
    //flags |= SQLITE_IOCAP_ATOMIC8K;
    //flags |= SQLITE_IOCAP_ATOMIC16K;
    //flags |= SQLITE_IOCAP_ATOMIC32K;
    //flags |= SQLITE_IOCAP_ATOMIC64K;
    //flags |= SQLITE_IOCAP_SAFE_APPEND; /* "The SQLITE_IOCAP_SAFE_APPEND value means that when data is appended to a file, the data is appended first then the size of the file is extended, never the other way around." */
    flags |= SQLITE_IOCAP_SEQUENTIAL; /* The SQLITE_IOCAP_SEQUENTIAL property means that information is written to disk in the same order as calls to xWrite(). */
    return flags;
}

static int cShmMap(sqlite3_file* baseFile, int iPg, int pgsz, int i, void volatile** v) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmMap(file = %s, iPg = %d, pgsz = %d, i = %d, v = <>)\n", file->zName, iPg, pgsz, i);
    #endif

    return SQLITE_IOERR;
}

static int cShmLock(sqlite3_file* baseFile, int offset, int n, int flags) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmLock(file = %s, offset = %d, n = %d, flags = [", file->zName, offset, n);

    /* these are the only valid flag combinations */
    if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_SHARED ) printf(" SHM_LOCK | SHM_SHARED ");
    if( flags == SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE ) printf(" SHM_LOCK | SHM_EXCLUSIVE ");
    if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED ) printf(" SHM_UNLOCK | SHM_SHARED ");
    if( flags == SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE ) printf(" SHM_UNLOCK | SHM_EXCLUSIVE ");

    printf("])\n");
    #endif

    return SQLITE_IOERR;
}

static void cShmBarrier(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmBarrier(file = %s)\n", file->zName);
    #endif
}

static int cShmUnmap(sqlite3_file* baseFile, int deleteFlag) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmUnmap(file = %s, deleteFlag = %d)\n", file->zName, deleteFlag);
    #endif

    return SQLITE_IOERR;
}

static int cFetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cFetch(file = %s, iOfst = %" PRIu64 ", iAmt = %d, pp = <>)\n", file->zName, iOfst, iAmt);
    #endif

    return SQLITE_IOERR;
}

static int cUnfetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, void *p) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cUnfetch(file = %s, iOfst = %" PRIu64 ", pp = <>)\n", file->zName, iOfst);
    #endif

    return SQLITE_IOERR;
}

/** sqlite3_vfs methods */

/* opens a file
 * @param vfs
 * @param zName the name of the file to open
 * @param baseFile the struct cFile to fill in
 * @param flags the set of requested OPEN flags; a set of flags from SQLITE_OPEN_*
 * @param pOutFlags the flags that were actually set
 */
static int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cOpen(vfs = <ptr>, zName = '%s', file = <not initialized>, flags = [", zName);
    if( flags & SQLITE_OPEN_MAIN_DB )        printf(" OPEN_MAIN_DB ");
    if( flags & SQLITE_OPEN_MAIN_JOURNAL )   printf(" OPEN_MAIN_JOURNAL ");
    if( flags & SQLITE_OPEN_TEMP_DB )        printf(" OPEN_TEMP_DB ");
    if( flags & SQLITE_OPEN_TEMP_JOURNAL )   printf(" OPEN_TEMP_JOURNAL ");
    if( flags & SQLITE_OPEN_TRANSIENT_DB )   printf(" OPEN_TRANSIENT_DB ");
    if( flags & SQLITE_OPEN_SUBJOURNAL )     printf(" OPEN_SUBJOURNAL ");
    if( flags & SQLITE_OPEN_MASTER_JOURNAL ) printf(" OPEN_MASTER_JOURNAL ");
    if( flags & SQLITE_OPEN_WAL )            printf(" OPEN_WAL ");
    //
    if( flags & SQLITE_OPEN_READWRITE )      printf(" OPEN_READWRITE ");
    if( flags & SQLITE_OPEN_CREATE )         printf(" OPEN_CREATE ");
    if( flags & SQLITE_OPEN_READONLY )       printf(" OPEN_READONLY ");
    //
    if( flags & SQLITE_OPEN_DELETEONCLOSE )  printf(" OPEN_DELETEONCLOSE ");
    if( flags & SQLITE_OPEN_EXCLUSIVE )      printf(" OPEN_EXCLUSIVE ");
    printf("])\n");
    #endif

    struct cFile* file = (struct cFile*)baseFile;
    file->composite_io_methods = 0;
    *pOutFlags = 0;

    /* does the file exist? */
    int fileExists = 0;
    cAccess(vfs, zName, SQLITE_ACCESS_EXISTS, &fileExists);

    if( flags & SQLITE_OPEN_CREATE && flags & SQLITE_OPEN_EXCLUSIVE ) {
        /* These two flags mean "that file should always be created, and that it is an error if it already exists."
         * They are always used together.
         */
         if( fileExists ) {
             return SQLITE_IOERR; //the file already exists -- error!
         }
    }

    FILE* fd = NULL;
    if( flags & SQLITE_OPEN_READWRITE ) {
        if( fileExists ) {
            fd = fopen(zName, "r+");
        } else {
            fd = fopen(zName, "w+");
        }

        *pOutFlags |= SQLITE_OPEN_READWRITE;
    } else if( flags & SQLITE_OPEN_READONLY ) {
        fd = fopen(zName, "r");
        *pOutFlags |= SQLITE_OPEN_READONLY;
    }

    if( fd == NULL ) {
        return SQLITE_IOERR;
    }
    
    file->composite_io_methods = &composite_io_methods;
    file->zName = zName;
    file->fd = fd;
    file->closed = 0;
    file->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE);
    return SQLITE_OK;
}

static int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cDelete(vfs = <ptr>, zName = %s, syncDir = %d)\n", zName, syncDir);
    #endif

    return SQLITE_ERROR;
}

/*
 * @param vfs
 * @param zName the name of the file or directory
 * @param flags the type of access check to perform; is SQLITE_ACCESS_EXISTS, _ACCESS_READWRITE, or _ACCESS_READ
 * @param pResOut
 */
static int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cAccess(vfs = <ptr>, zName = %s, flags = [", zName);
    if( flags == SQLITE_ACCESS_EXISTS ) printf(" ACCESS_EXISTS ");
    else if( flags == SQLITE_ACCESS_READWRITE ) printf(" ACCESS_READWRITE ");
    else if( flags == SQLITE_ACCESS_READ ) printf(" ACCESS_READ ");
    printf("], pResOut = <flags>)\n");
    #endif

    if( flags == SQLITE_ACCESS_EXISTS ) {
        *pResOut = (access(zName, F_OK) != -1);
    } else if( flags == SQLITE_ACCESS_READWRITE ) {
        *pResOut = (access(zName, R_OK | W_OK) != -1);
    } else if( flags == SQLITE_ACCESS_READ ) {
        *pResOut = (access(zName, R_OK) != -1);
    }

    return SQLITE_OK;
}

static int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cFullPathname(vfs = <ptr>, zName = %s, nOut = %d, zOut = %s)\n", zName, nOut, zOut);
    #endif

    static const char* full_pathname = "/tmp/";
    const size_t full_pathname_len = strlen(full_pathname);

    int i = 0;
    for( ; i < full_pathname_len; i++ ) zOut[i] = full_pathname[i];
    for( ; i < full_pathname_len + strlen(zName); i++ ) zOut[i] = zName[i - full_pathname_len];
    zOut[i] = 0;

    //assert( i < nOut );
    return SQLITE_OK;
}

/* attempts to return nByte bytes of randomness.
 * @return the actual number of bytes of randomness generated
 */
static int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cRandomness(vfs = <ptr>, nByte = %d, zOut = <ptr>)\n", nByte);
    #endif

    struct composite_vfs_data *data = (struct composite_vfs_data*)vfs->pAppData;
    return fread(zOut, 1, nByte, data->random_fd);
}

/* sleep for at least the given number of microseconds
 */
static int cSleep(sqlite3_vfs* vfs, int microseconds) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cSleep(vfs = <vfs>, microseconds = %d)\n", microseconds);
    #endif
}

static int cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cGetLastError(i = %d, ch = %s)\n", i, ch);
    #endif
}

/* "returns a Julian Day Number for the current date and time as a floating point value"
 */
static int cCurrentTime(sqlite3_vfs* vfs, double* time) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cCurrentTime()\n");
    #endif
}

/* "returns, as an integer, the Julian Day Number multiplied by 86400000 (the number of milliseconds in a 24-hour day)"
 */
static int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cCurrentTimeInt64()\n");
    #endif
}

static int cMutexInit() {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexInit()\n");
    #endif
}

static int cMutexEnd() {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexEnd()\n");
    #endif
}

/* creates a mutex of the given type
 * @param mutexType one of SQLITE_MUTEX_*
 * @return a pointer to a mutex, or NULL if it couldn't be created
 */
static sqlite3_mutex* cMutexAlloc(int mutexType) {
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

static void cMutexFree(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexFree(mutex)\n");
    #endif

}

/* tries to enter the given mutex.
 * if another thread is in the mutex, this method will block.
 */
static void cMutexEnter(sqlite3_mutex *mutex) {
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
static int cMutexTry(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexTry(mutex)\n");
    #endif

}

/* exits the given mutex.
 * behavior is undefined if the mutex wasn't entered by the calling thread
 */
static void cMutexLeave(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexLeave(mutex)\n");
    #endif

}

/* returns true if this mutex is held by the calling thread
 *
 * this is used only in SQLite assert()'s, so a working
 * implementation isn't really needed; this can just return TRUE
 */
static int cMutexHeld(sqlite3_mutex *mutex) {
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
static int cMutexNotheld(sqlite3_mutex *mutex) {
    #if SQLITE_COS_PROFILE_MUTEX
    printf("cMutexNotheld(mutex)\n");
    #endif

    return 1;
}

/* Memory allocation function */
static void *cMemMalloc(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemMalloc(sz = %d)\n", sz);
    #endif

    return malloc(sz);
}

/* Free a prior allocation */
static void cMemFree(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemFree(mem = <ptr>)\n");
    #endif

    free(mem);
}

/* Resize an allocation */
static void *cMemRealloc(void* mem, int newSize) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemRealloc(mem = <ptr>, newSize = %d)\n", newSize);
    #endif

    return realloc(mem, newSize);
}

/* Return the size of an allocation */
static int cMemSize(void* mem) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemSize(mem = <ptr>)\n");
    #endif
    return sizeof(mem);
}

/* Round up request size to allocation size */
static int cMemRoundup(int sz) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemRoundup(sz = %d)\n", sz);
    #endif

    return sz;
}

/* Initialize the memory allocator */
static int cMemInit(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemInit(pAppData = %p)\n", pAppData);
    #endif

    return SQLITE_OK;
}

/* Deinitialize the memory allocator */
static void cMemShutdown(void* pAppData) {
    #if SQLITE_COS_PROFILE_MEMORY
    printf("cMemShutdown(pAppData = %p)\n", pAppData);
    #endif
}

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
