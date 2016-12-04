#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <stdio.h>
#include <inttypes.h>

#include "os_composite.h"

/* sqlite3_io_methods */
static int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cClose(file = '%s')\n", file->zName);
}

static int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cRead(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")\n", file->zName, iAmt, iOfst);
    return SQLITE_IOERR;
}

static int cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cWrite(file = %s, buf = <>, iAmt = %d, iOfst = %" PRIu64 ")\n", file->zName, iAmt, iOfst);
    return SQLITE_IOERR;
}

static int cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cTruncate(file = %s, size = %" PRIu64 ")\n", file->zName, size);
    return SQLITE_IOERR;
}

static int cSync(sqlite3_file* baseFile, int flags) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cTruncate(file = %s, flags = [???])\n", file->zName);
    return SQLITE_IOERR;
}

static int cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cFileSize(file = %s)\n", file->zName);
    return SQLITE_IOERR;
}

/* increases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
static int cLock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cLock(file = %s, lockType = [", file->zName);
    if( lockType & SQLITE_LOCK_NONE )      printf(" LOCK_NONE ");
    if( lockType & SQLITE_LOCK_SHARED )    printf(" LOCK_SHARED ");
    if( lockType & SQLITE_LOCK_RESERVED )  printf(" LOCK_RESERVED ");
    if( lockType & SQLITE_LOCK_PENDING )   printf(" LOCK_PENDING ");
    if( lockType & SQLITE_LOCK_EXCLUSIVE ) printf(" LOCK_EXCLUSIVE ");
    printf("])\n");
    return SQLITE_IOERR;
}

/* decreases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
static int cUnlock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;
    printf("cUnlock(file = %s, lockType = [", file->zName);
    if( lockType & SQLITE_LOCK_NONE )      printf(" LOCK_NONE ");
    if( lockType & SQLITE_LOCK_SHARED )    printf(" LOCK_SHARED ");
    if( lockType & SQLITE_LOCK_RESERVED )  printf(" LOCK_RESERVED ");
    if( lockType & SQLITE_LOCK_PENDING )   printf(" LOCK_PENDING ");
    if( lockType & SQLITE_LOCK_EXCLUSIVE ) printf(" LOCK_EXCLUSIVE ");
    printf("])\n");
    return SQLITE_IOERR;
}

/* returns true if any connection has a RESERVED, PENDING, or EXCLUSIVE lock on this file
 */
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

/* opens a file
 * @param vfs
 * @param zName the name of the file to open
 * @param baseFile the struct cFile to fill in
 * @param flags the set of requested OPEN flags; a set of flags from SQLITE_OPEN_*
 * @param pOutFlags the flags that were actually set
 */
static int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
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

    struct cFile* file = (struct cFile*)baseFile;
    file->composite_io_methods = &composite_io_methods;
    file->zName = zName;
    return SQLITE_OK;
}

static int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    printf("cDelete(vfs = <ptr>, zName = %s, syncDir = %d)\n", zName, syncDir);
    return SQLITE_ERROR;
}

/*
 * @param vfs
 * @param zName the name of the file or directory
 * @param flags the type of access check to perform; is SQLITE_ACCESS_EXISTS, _ACCESS_READWRITE, or _ACCESS_READ
 * @param pResOut
 */
static int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    printf("cAccess(vfs = <ptr>, zName = %s, flags = %d, pResOut = <flags>)\n", zName, flags);
    return SQLITE_ERROR;
}

static int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    printf("cFullPathname(vfs = <ptr>, zName = %s, nOut = %d, zOut = %s)\n", zName, nOut, zOut);
    zOut = "/tmp/defaultCFullPathname";
    return SQLITE_OK;
}

/* attempts to return nByte bytes of randomness.
 * @return the actual number of bytes of randomness generated
 */
static int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    printf("cRandomness(vfs = <ptr>, nByte = %d, zOut = <ptr>)\n", nByte);
    return 0;
}

/* sleep for at least the given number of microseconds
 */
static int cSleep(sqlite3_vfs* vfs, int microseconds) {}

static int cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    printf("cGetLastError()\n");
}

/* "returns a Julian Day Number for the current date and time as a floating point value"
 */
static int cCurrentTime(sqlite3_vfs* vfs, double* time) {
    printf("cCurrentTime()\n");
}

/* "returns, as an integer, the Julian Day Number multiplied by 86400000 (the number of milliseconds in a 24-hour day)"
 */
static int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    printf("cCurrentTimeInt64()\n");
}

#endif