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
#include <errno.h>

#include "os_composite.h"

/* sqlite3_io_methods */
int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;

    if( close(file->fd) != 0 ) {
        return SQLITE_IOERR_CLOSE;
    }

    file->closed = 1;

    //delete this file if necessary
    if( file->deleteOnClose ) {
        if( remove(file->zName) != 0 ) {
            return SQLITE_IOERR_DELETE;
        }
    }

    return SQLITE_OK;
}

int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;

    /* seek to the correct position */
    if( lseek(file->fd, iOfst, SEEK_SET) != 0 ) {
        return SQLITE_IOERR_SEEK;
    }

    /* read the bytes */
    int bytesRead = read(file->fd, buf, iAmt);

    /* was there an error? */
    if( bytesRead == -1 ) {
        if( errno == EOVERFLOW ) {
            /* if we do a short read, we have to fill the rest of the buffer with 0's */
            int i;
            for( i = bytesRead; i < iAmt; i++ )
                ((char*)buf)[i] = 0;

            return SQLITE_IOERR_SHORT_READ;
        } else {
            return SQLITE_IOERR_READ;
        }
    }

    if( bytesRead < iAmt ) {
        /* if we do a short read, we have to fill the rest of the buffer with 0's */
        int i;
        for( i = bytesRead; i < iAmt; i++ )
            ((char*)buf)[i] = 0;

        return SQLITE_IOERR_SHORT_READ;
    }

    //assert( bytesRead == iAmt);
    return SQLITE_OK;
}

int cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;

    /* seek to the correct position */
    if( lseek(file->fd, iOfst, SEEK_SET) != 0 ) {
        return SQLITE_IOERR_SEEK;
    }

    /* write the bytes */
    int bytesWritten = write(file->fd, buf, iAmt);
    if( bytesWritten == iAmt ) {
        return SQLITE_OK;
    }

    return SQLITE_IOERR_WRITE;
}

int cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    struct cFile* file = (struct cFile*)baseFile;
    //TODO this is a NOP
    return SQLITE_OK;
}

int cSync(sqlite3_file* baseFile, int flags) {
    struct cFile* file = (struct cFile*)baseFile;

    int res = 0;
    if( flags & SQLITE_SYNC_FULL || flags & SQLITE_SYNC_NORMAL ) {
        res = fsync( file->fd );
    } else {
        res = fdatasync( file->fd );
    }

    if( res != 0 ) {
        return SQLITE_IOERR_FSYNC;
    }

    return SQLITE_OK;
}

int cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    struct cFile* file = (struct cFile*)baseFile;

    /* seek to the end, then ask for the position */
    off_t pos = lseek(file->fd, 0L, SEEK_END);

    if( pos == -1 ) {
        return SQLITE_IOERR_SEEK;
    }

    *pSize = pos;

    return SQLITE_OK;
}

/* increases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
int cLock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;
    return SQLITE_OK;
}

/* decreases the lock on a file
 * @param lockType one of SQLITE_LOCK_*
 */
int cUnlock(sqlite3_file* baseFile, int lockType) {
    struct cFile* file = (struct cFile*)baseFile;
    return SQLITE_OK;
}

/* returns true if any connection has a RESERVED, PENDING, or EXCLUSIVE lock on this file
 */
int cCheckReservedLock(sqlite3_file* baseFile, int *pResOut) {
    struct cFile* file = (struct cFile*)baseFile;
    if( pResOut ) *pResOut = 0;
    return SQLITE_OK;
}

/* "VFS implementations should return [SQLITE_NOTFOUND] for file control opcodes that they do not recognize."
 */
int cFileControl(sqlite3_file* baseFile, int op, void *pArg) {
    return SQLITE_NOTFOUND;
}

/* "The xSectorSize() method returns the sector size of the device that underlies the file."
 */
int cSectorSize(sqlite3_file* baseFile) {
    return 512; //TODO
}

/* "The xDeviceCharacteristics() method returns a bit vector describing behaviors of the underlying device"
 */
int cDeviceCharacteristics(sqlite3_file* baseFile) {
    int flags = 0;
    //flags |= SQLITE_IOCAP_ATOMIC; /* "The SQLITE_IOCAP_ATOMIC property means that all writes of any size are atomic." */
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
    return SQLITE_IOERR;
}

int cShmLock(sqlite3_file* baseFile, int offset, int n, int flags) {
    return SQLITE_OK;
}

void cShmBarrier(sqlite3_file* baseFile) {
}

int cShmUnmap(sqlite3_file* baseFile, int deleteFlag) {
    return SQLITE_IOERR;
}

int cFetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, int iAmt, void **pp) {
    return SQLITE_IOERR;
}

int cUnfetch(sqlite3_file* baseFile, sqlite3_int64 iOfst, void *p) {
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

    if( pOutFlags ) *pOutFlags = flags;

    /* does the file exist? */
    int fileExists = 0;
    cAccess(vfs, zName, SQLITE_ACCESS_EXISTS, &fileExists);

    if( (flags & SQLITE_OPEN_CREATE) && (flags & SQLITE_OPEN_EXCLUSIVE) ) {
        /* These two flags mean "that file should always be created, and that it is an error if it already exists."
         * They are always used together.
         */
         if( fileExists ) {
             return SQLITE_IOERR; //the file already exists -- error!
         }
    }

    int fd = -1;
    if( flags & SQLITE_OPEN_READWRITE ) {
        if( fileExists ) {
            fd = open(zName, O_RDWR /*"r+b"*/);
        } else {
            fd = open(zName, O_RDWR | O_CREAT /*"w+b"*/);
        }
    } else if( flags & SQLITE_OPEN_READONLY ) {
        fd = open(zName, O_RDONLY);
    }

    if( fd == -1 ) {
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

    /* make sure the file exists */
    int exists = 0;
    cAccess(vfs, zName, SQLITE_ACCESS_EXISTS, &exists);
    if( !exists ) {
        return SQLITE_OK;
    }

    /* delete the file */
    if( remove(zName) != 0 ) {
        return SQLITE_IOERR_DELETE;
    }

    return SQLITE_OK;
}

/*
 * @param vfs
 * @param zName the name of the file or directory
 * @param flags the type of access check to perform; is SQLITE_ACCESS_EXISTS, _ACCESS_READWRITE, or _ACCESS_READ
 * @param pResOut
 */
int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
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
    struct composite_vfs_data *data = (struct composite_vfs_data*)vfs->pAppData;
    int bytes_read = read(data->random_fd, zOut, nByte);
    if( bytes_read < 0 )
        bytes_read = 0;
    return bytes_read;
}

/* sleep for at least the given number of microseconds
 */
int cSleep(sqlite3_vfs* vfs, int microseconds) {
    return SQLITE_OK;
}

int cGetLastError(sqlite3_vfs* vfs, int i, char *ch) {
    return SQLITE_OK;
}

/* "returns a Julian Day Number for the current date and time as a floating point value"
 */
int cCurrentTime(sqlite3_vfs* vfs, double* time) {
    //TODO
}

/* "returns, as an integer, the Julian Day Number multiplied by 86400000 (the number of milliseconds in a 24-hour day)"
 */
int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time) {
    //TODO
}

#endif // SQLITE_OS_OTHER