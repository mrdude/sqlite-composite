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

#define PAGE_SIZE 4096 /* assume a 4096 byte page-size */
#define INITIAL_BUF_DATA_SIZE (PAGE_SIZE*2)

/* the maximum possible size of a file; the largest value that can be represented with a signed 64-bit int
 * on 32-bit systems, the actual minimum will be much lower.
 */
#define MAX_FILE_LEN ( (int64_t)(1L<<(sizeof(int64_t)-1)) )

#define MIN(type, a, b) ( ((type)(a)) < ((type)(b)) ? (type)(a) : (type)(b) )

/* inmem fs structs */
struct fs_data {
    char* buf; /* a pointer to the buffer containing the file's data */
    int64_t len; /* the portion of this buffer contains valid data */
};

struct fs_file {
    struct composite_vfs_data* cVfs;
    struct fs_file* next; /* the next file in the list */

    const char* zName; /* the name of the file */
    struct fs_data data;
    int ref; /* the number of open cFile's the file has */
};

static struct fs_file* _fs_file_list = 0;

/* private inmem fs functions */
static void* _FS_MALLOC(int sz) {
    sz = composite_mem_methods.xRoundup(sz);
    return composite_mem_methods.xMalloc(sz);
}

static void* _FS_REALLOC(void* mem, int newSize) {
    return composite_mem_methods.xRealloc(mem, newSize);
}

static void _FS_FREE(void* mem) {
    composite_mem_methods.xFree(mem);
}

/* finds the file with the given name, or 0 if it doesn't exist */
static struct fs_file* _fs_find_file(sqlite3_vfs* vfs, const char* zName) {
    printf("_fs_find_file()\n");
    struct fs_file* file;
    for( file = _fs_file_list; file != 0; file = file->next ) {
        if( strncmp(file->zName, zName, MAX_PATHNAME) == 0 ) {
            return file;
        }
    }
    return 0;
}

static struct fs_file* _fs_file_alloc(sqlite3_vfs* vfs, const char *zName) {
    printf("_fs_file_alloc()\n");
    struct composite_vfs_data* cVfs = (struct composite_vfs_data*)(vfs->pAppData);
    struct fs_file* file = _FS_MALLOC( sizeof(struct fs_file) );
    if( file == 0 )
        return 0;
    
    char* buf = _FS_MALLOC( INITIAL_BUF_DATA_SIZE );
    if( buf == 0 )
        return 0;
    
    file->cVfs = cVfs;
    file->next = 0;
    file->zName = zName;
    file->data.buf = buf;
    file->data.len = 0;
    file->ref = 0;
    return file;
}

/* free the file and all of its blocks */
static void _fs_file_free(struct fs_file* file) {
    printf("_fs_file_free()\n");
    _FS_FREE( file->data.buf );
    _FS_FREE( file );
}

/* makes sure that there is sz bytes of space in file's data buffer
 * returns 1 on success, 0 on failure
 */
static int _fs_data_ensure_capacity(struct fs_file* file, int64_t sz) {
    //TODO
}

/* inmem fs functions */
static struct fs_file* fs_open(sqlite3_vfs* vfs, const char* zName) {
    printf("fs_open()\n");
    //TODO make this atomic
    //atomic {
    struct fs_file* file = _fs_find_file(vfs, zName);
    if( file == 0 ) {
        file = _fs_file_alloc(vfs, zName);
    }
    //}
    if( file == 0 ) {
        return 0;
    }

    file->ref++;
    return file;
}

static void fs_close(struct fs_file* file) {
    printf("fs_close()");
    //TODO make this threadsafe
    file->ref--;
    if( file->ref == 0 ) {
        _fs_file_free(file);
        file = 0;
    }
}

/* returns the number of bytes read, or -1 if an error occurred. short reads are allowed. */
static int fs_read(struct fs_file* file, int64_t offset, int len, void* buf) {
    printf("fs_read()");
    //TODO check locks

    /* perform sanity checks on offset and len */
    if( offset < 0 || len < 0 ) {
        return -1;
    }
    
    /* determine the number of bytes to read */
    int64_t end_offset = offset + (int64_t)len;
    if( end_offset > file->data.len ) end_offset = file->data.len;
    int bytes_read = (int)(end_offset - offset);
    printf("\t=> bytes_read = %d\n", bytes_read);

    /* copy the bytes into the buffer */
    if( bytes_read > 0 ) {
        strncpy( (char*)buf, (const char*)(&file->data.buf[offset]), (size_t)bytes_read);
    }

    return bytes_read;
}

/* returns the number of bytes written, or -1 if an error occurred. partial writes are not allowed. */
static int fs_write(struct fs_file* file, int64_t offset, int len, const void* buf) {
    printf("fs_write()");
    //TODO check locks -- this should occur atomically

    /* perform sanity checks on offset and len */
    if( offset < 0 || len < 0 ) {
        return -1;
    }

    /* ensure that our buffer is large enough to perform the write */
    int64_t end_offset = offset + (int64_t)len;
    if( _fs_data_ensure_capacity(file, end_offset) == 0 ) {
        return -1; /* we don't have enough memory to perform the write */
    }

    /* perform the write */
    strncpy( (char*)(&file->data.buf[offset]), (const char*)buf, (size_t)len );

    /* adjust file->data.len */
    file->data.len = end_offset;

    return len;
}

/* sqlite3_io_methods */
int cClose(sqlite3_file* baseFile) {
    struct cFile* file = (struct cFile*)baseFile;
    
    fs_close((struct fs_file*)file->fd);
    file->fd = 0;

    file->closed = 1;

    return SQLITE_OK;
}

int cRead(sqlite3_file* baseFile, void* buf, int iAmt, sqlite3_int64 iOfst) {
    struct cFile* file = (struct cFile*)baseFile;
    struct fs_file* fd = (struct fs_file*)file->fd;

    /* read the bytes */
    int bytesRead = fs_read(fd, iOfst, iAmt, buf);

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
    //TODO this is a NOP
    return SQLITE_OK;
}

int cSync(sqlite3_file* baseFile, int flags) {
    struct cFile* file = (struct cFile*)baseFile;
    //TODO this is a NOP
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
    return SQLITE_NOTFOUND;
}

/* "The xSectorSize() method returns the sector size of the device that underlies the file."
 */
int cSectorSize(sqlite3_file* baseFile) {
    return PAGE_SIZE;
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

    flags |= SQLITE_OPEN_DELETEONCLOSE; /* all files are delete on close */

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
    file->closed = 0;
    file->deleteOnClose = (flags & SQLITE_OPEN_DELETEONCLOSE);
    return SQLITE_OK;
}

int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir) {
    /* files are simply deleted when no one has them open */
    return SQLITE_OK;
}

/*
 * @param vfs
 * @param zName the name of the file or directory
 * @param flags the type of access check to perform; is SQLITE_ACCESS_EXISTS, _ACCESS_READWRITE, or _ACCESS_READ
 * @param pResOut
 */
int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut) {
    /* all files can be accessed by everyone */
    //TODO restrict access based on Composite caps
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

/* attempts to return nByte bytes of randomness.
 * @return the actual number of bytes of randomness generated
 */
int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut) {
    struct composite_vfs_data *data = (struct composite_vfs_data*)vfs->pAppData;

    int total_bytes_written = 0;
    while(1) {
        int bytes_read = read(data->random_fd, &zOut[total_bytes_written], nByte - total_bytes_written);
        if( bytes_read < 0 )
            break;

        total_bytes_written += bytes_read;
    }
    
    return total_bytes_written;
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