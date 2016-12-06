#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include "os_composite.h"

/* sqlite3_io_methods */
int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;
    
    fs_close((struct fs_file*)file->fd);
    file->fd = 0;

    return SQLITE_OK;
}

int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd = (struct fs_file*)file->fd;

    /* read the bytes */
    int bytesRead = fs_read(fd, iOfst, iAmt, buf);

    /* was there an error? */
    if( bytesRead == -1 ) {
        return SQLITE_IOERR_READ;
    }

    if( bytesRead < iAmt ) {
        /* if we do a short read, we have to fill the rest of the buffer with 0's */
        char* data = buf;
        int i;
        for( i = bytesRead; i < iAmt; i++ )
            data[i] = 0;

        return SQLITE_IOERR_SHORT_READ;
    }

    //assert( bytesRead == iAmt);
    return SQLITE_OK;
}

int cWrite(sqlite3_file* baseFile, const void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd = (struct fs_file*)file->fd;

    int bytesWritten = fs_write(fd, iOfst, iAmt, buf);
    if( bytesWritten == iAmt ) {
        return SQLITE_OK;
    }

    return SQLITE_IOERR_WRITE;
}

int cTruncate(sqlite3_file* baseFile, sqlite3_int64 size) {
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd = (struct fs_file*)file->fd;
    if( fs_truncate(fd, size) ) {
        return SQLITE_OK;
    } else {
        return SQLITE_IOERR_TRUNCATE;
    }
}

int cSync(sqlite3_file* baseFile, int flags) {
    struct cFile* file = (struct cFile*)baseFile;
    /* this is a NOP -- writes to the in-memory filesystem are atomic */
    return SQLITE_OK;
}

int cFileSize(sqlite3_file* baseFile, sqlite3_int64 *pSize) {
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd = (struct fs_file*)file->fd;

    *pSize = fd->data.len;
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
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd;

    switch( op ) {
        case SQLITE_FCNTL_SIZE_HINT:
            /* "[SQLITE_FCNTL_SIZE_HINT] opcode is used by SQLite to give the VFS layer a hint
             * of how large the database file will grow to be during the current transaction...the
             * underlying VFS might choose to preallocate database file space based on this hint..."
             */
             if( file->fd ) {
                 fd = (struct fs_file*)file->fd;
                 int size_hint = *((int *)pArg);
                 fs_size_hint(fd, (sqlite3_int64)size_hint);
             }
             return SQLITE_OK;
        default:
            return SQLITE_NOTFOUND;
    }
}

/* "The xSectorSize() method returns the sector size of the device that underlies the file."
 */
int cSectorSize(sqlite3_file* baseFile) {
    return FS_SECTOR_SIZE;
}

/* "The xDeviceCharacteristics() method returns a bit vector describing behaviors of the underlying device"
 */
int cDeviceCharacteristics(sqlite3_file* baseFile) {
    int flags = 0;
    flags |= SQLITE_IOCAP_ATOMIC; /* "The SQLITE_IOCAP_ATOMIC property means that all writes of any size are atomic." */
    flags |= SQLITE_IOCAP_ATOMIC512; /* "The SQLITE_IOCAP_ATOMICnnn values mean that writes of blocks that are nnn bytes in size and are aligned to an address which is an integer multiple of nnn are atomic." */
    flags |= SQLITE_IOCAP_ATOMIC1K;
    flags |= SQLITE_IOCAP_ATOMIC2K;
    flags |= SQLITE_IOCAP_ATOMIC4K;
    flags |= SQLITE_IOCAP_ATOMIC8K;
    flags |= SQLITE_IOCAP_ATOMIC16K;
    flags |= SQLITE_IOCAP_ATOMIC32K;
    flags |= SQLITE_IOCAP_ATOMIC64K;
    flags |= SQLITE_IOCAP_SAFE_APPEND; /* "The SQLITE_IOCAP_SAFE_APPEND value means that when data is appended to a file, the data is appended first then the size of the file is extended, never the other way around." */
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

    void* fd = fs_open(vfs, zName);
    if( fd == 0 ) {
        return SQLITE_IOERR;
    }
    
    file->composite_io_methods = &composite_io_methods;
    file->zName = zName;
    file->fd = fd;
    if( flags & SQLITE_OPEN_DELETEONCLOSE ) {
        fs_delete(vfs, zName); /* the file will be deleted when it's reference count hits 0 */
    }

    return SQLITE_OK;
}

int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    if( fs_delete(vfs, zName) ) {
        return SQLITE_OK;
    } else {
        return SQLITE_IOERR_DELETE;
    }
}

/*
 * @param vfs
 * @param zName the name of the file or directory
 * @param flags the type of access check to perform; is SQLITE_ACCESS_EXISTS, _ACCESS_READWRITE, or _ACCESS_READ
 * @param pResOut
 */
int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    /* all files can be accessed by everyone as long as it exists */
    *pResOut = fs_exists(vfs, zName);
    return SQLITE_OK;
}

int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut) {
    const int zNameLen = strnlen(zName, MAX_PATHNAME);
    if( nOut < zNameLen ) /* these isn't enough room to copy the string */
        return SQLITE_CANTOPEN;
    
    int i;
    for( i = 0; i < zNameLen; i++ ) {
        zOut[i] = zName[i];
    }
    zOut[i] = 0;

    return SQLITE_OK;
}

/* xorshift* */
static sqlite3_uint64 get_random(sqlite3_uint64 *state) {
    const sqlite3_uint64 magic = 2685821657736338717L;
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 17;
    return (*state) * magic;
}

/* attempts to return nByte bytes of randomness.
 * @return the actual number of bytes of randomness generated
 */
int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    //TODO how to get this from composite?
    /* cRandom uses the prng defined in get_random() to get random bytes
     */
    struct composite_vfs_data *data = (struct composite_vfs_data*)vfs->pAppData;

    int i = 0;
    for( ; i + 8 < nByte; i += 8 ) {
        sqlite3_uint64 rand = get_random( &data->prng_state );
        *((sqlite3_uint64*)&zOut[i]) = rand;
    }

    for( ; i + 4 < nByte; i += 4 ) {
        int rand = (int)get_random( &data->prng_state );
        *((int*)&zOut[i]) = rand;
    }

    for( ; i < nByte; i++ ) {
        char rand = (char)get_random( &data->prng_state );
        zOut[i] = rand;
    }

    /* return the number of bytes generated */
    return nByte;
}

/* sleep for at least the given number of microseconds
 */
int cSleep(sqlite3_vfs* vfs, int microseconds) {
    //TODO
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

void cVfsInit() {
    fs_init();
}

void cVfsDeinit() {
    fs_deinit();
}

#endif // SQLITE_OS_OTHER