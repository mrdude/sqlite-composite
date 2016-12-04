#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_composite.h"

/* sqlite3_io_methods */
int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;

    fclose(file->fd);
    file->closed = 1;

    //TODO delete?
}

int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;

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

int cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    return SQLITE_IOERR;
}

int cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    struct cFile* file = (struct cFile*)baseFile;
    return SQLITE_IOERR;
}

int cSync(sqlite3_file* baseFile, int flags) {
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

int cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cFileSize(file = %s)\n", file->zName);
    #endif

    return SQLITE_IOERR;
}

/* increases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
int cLock(sqlite3_file* baseFile, int lockType) {
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
int cUnlock(sqlite3_file* baseFile, int lockType) {
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
int cCheckReservedLock(sqlite3_file* baseFile, int *pResOut) {
    struct cFile* file = (struct cFile*)baseFile;

    #if SQLITE_COS_PROFILE_VFS
    printf("cCheckReservedLock(file = %s)\n", file->zName);
    #endif

    *pResOut = 0;

    return SQLITE_OK;
}

/* "VFS implementations should return [SQLITE_NOTFOUND] for file control opcodes that they do not recognize."
 */
int cFileControl(sqlite3_file* baseFile, int op, void *pArg) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cFileControl(file = %s, op = %d, pArg = <>)\n", file->zName, op);
    #endif

    return SQLITE_NOTFOUND;
}

/* "The xSectorSize() method returns the sector size of the device that underlies the file."
 */
int cSectorSize(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cSectorSize(file = %s)\n", file->zName);
    #endif

    return 512; //TODO
}

/* "The xDeviceCharacteristics() method returns a bit vector describing behaviors of the underlying device"
 */
int cDeviceCharacteristics(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cDeviceCharacteristics(file = %s)\n", file->zName);
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

int cShmMap(sqlite3_file* baseFile, int iPg, int pgsz, int i, void volatile** v) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmMap(file = %s, iPg = %d, pgsz = %d, i = %d, v = <>)\n", file->zName, iPg, pgsz, i);
    #endif

    return SQLITE_IOERR;
}

int cShmLock(sqlite3_file* baseFile, int offset, int n, int flags) {
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

void cShmBarrier(sqlite3_file* baseFile) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmBarrier(file = %s)\n", file->zName);
    #endif
}

int cShmUnmap(sqlite3_file* baseFile, int deleteFlag) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cShmUnmap(file = %s, deleteFlag = %d)\n", file->zName, deleteFlag);
    #endif

    return SQLITE_IOERR;
}

int cFetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
    #if SQLITE_COS_PROFILE_VFS
    struct cFile* file = (struct cFile*)baseFile;
    printf("cFetch(file = %s, iOfst = %" PRIu64 ", iAmt = %d, pp = <>)\n", file->zName, iOfst, iAmt);
    #endif

    return SQLITE_IOERR;
}

int cUnfetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, void *p) {
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
int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags) {
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

int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
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
int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
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

int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cFullPathname(vfs = <ptr>, zName = %s, nOut = %d, zOut = %s)\n", zName, nOut, zOut);
    #endif

    const char* full_pathname = "/tmp/";
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
int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cRandomness(vfs = <ptr>, nByte = %d, zOut = <ptr>)\n", nByte);
    #endif

    struct composite_vfs_data *data = (struct composite_vfs_data*)vfs->pAppData;
    return fread(zOut, 1, nByte, data->random_fd);
}

/* sleep for at least the given number of microseconds
 */
int cSleep(sqlite3_vfs* vfs, int microseconds) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cSleep(vfs = <vfs>, microseconds = %d)\n", microseconds);
    #endif
}

int cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cGetLastError(i = %d, ch = %s)\n", i, ch);
    #endif
}

/* "returns a Julian Day Number for the current date and time as a floating point value"
 */
int cCurrentTime(sqlite3_vfs* vfs, double* time) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cCurrentTime()\n");
    #endif
}

/* "returns, as an integer, the Julian Day Number multiplied by 86400000 (the number of milliseconds in a 24-hour day)"
 */
int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    #if SQLITE_COS_PROFILE_VFS
    printf("cCurrentTimeInt64()\n");
    #endif
}

#endif // SQLITE_OS_OTHER