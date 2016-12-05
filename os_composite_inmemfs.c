/* Contains code for an in-memory filesystem implementation */

#include "sqlite3.h"
#if SQLITE_OS_OTHER

#include "os_composite.h"

#define PAGE_SIZE 4096 /* assume a 4096 byte page-size */
#define INITIAL_BUF_DATA_SIZE (PAGE_SIZE*2)

/* the maximum possible size of a file; the largest value that can be represented with a signed 64-bit int
 * on 32-bit systems, the actual minimum will be much lower.
 */
#define MAX_FILE_LEN ( (int64_t)(1L<<(sizeof(int64_t)-1)) )

#define MIN(type, a, b) ( ((type)(a)) < ((type)(b)) ? (type)(a) : (type)(b) )

static struct fs_file* _fs_file_list;

/* private inmem fs functions */
static void* _FS_MALLOC(int sz) {
    sz = composite_mem_methods.xRoundup(sz);
    return composite_mem_methods.xMalloc(sz);
}

static int _FS_MEMSIZE(void* mem) {
    return composite_mem_methods.xSize(mem);
}

static void* _FS_REALLOC(void* mem, int newSize) {
    return composite_mem_methods.xRealloc(mem, newSize);
}

static void _FS_FREE(void* mem) {
    composite_mem_methods.xFree(mem);
}

/* finds the file with the given name, or 0 if it doesn't exist */
static struct fs_file* _fs_find_file(sqlite3_vfs* vfs, const char* zName) {
    printf("_fs_find_file(%s)\n", zName);
    struct fs_file* file;
    for( file = _fs_file_list; file != 0; file = file->next ) {
        if( strncmp(file->zName, zName, MAX_PATHNAME) == 0 ) {
            return file;
        }
    }
    return 0;
}

static struct fs_file* _fs_file_alloc(sqlite3_vfs* vfs, const char *zName) {
    printf("_fs_file_alloc(%s)\n", zName);
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
    printf("_fs_file_free(%s)\n", file->zName);
    _FS_FREE( file->data.buf );
    _FS_FREE( file );
}

/* makes sure that there is sz bytes of space in file's data buffer
 * returns 1 on success, 0 on failure
 */
static int _fs_data_ensure_capacity(struct fs_file* file, int64_t sz) {
    printf("_fs_data_ensure_capacity(%s, sz=%" PRIu64 ")\n", file->zName, sz);
    int old_size = _FS_MEMSIZE(file->data.buf);
    if( old_size < sz ) {
        int new_size = old_size * 2;
        if( new_size < sz ) {
            new_size = sz;
        }

        void* new_buf = _FS_REALLOC(file->data.buf, new_size);
        if( new_buf == 0 ) {
            return 0;
        } else {
            file->data.buf = new_buf;
        }
    }

    return 1;
}

/* inmem fs functions */
void fs_init() {
}

void fs_deinit() {
}

struct fs_file* fs_open(sqlite3_vfs* vfs, const char* zName) {
    printf("fs_open(%s)\n", zName);
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

void fs_close(struct fs_file* file) {
    printf("fs_close(%s)\n", file->zName);
    //TODO make this threadsafe
    file->ref--;
}

/* returns the number of bytes read, or -1 if an error occurred. short reads are allowed. */
int fs_read(struct fs_file* file, int64_t offset, int len, void* buf) {
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

    /* copy the bytes into the buffer */
    if( bytes_read > 0 ) {
        strncpy( (char*)buf, (const char*)(&file->data.buf[offset]), (size_t)bytes_read);
    }

    printf("\t=> bytes_read = %d\n", bytes_read);
    return bytes_read;
}

/* returns the number of bytes written, or -1 if an error occurred. partial writes are not allowed. */
int fs_write(struct fs_file* file, int64_t offset, int len, const void* buf) {
    printf("fs_write()");
    //TODO check locks -- this should occur atomically

    /* perform sanity checks on offset and len */
    if( offset < 0 || len < 0 ) {
        printf("\t=> failed sanity check\n");
        return -1;
    }

    /* ensure that our buffer is large enough to perform the write */
    int64_t end_offset = offset + (int64_t)len;
    if( _fs_data_ensure_capacity(file, end_offset) == 0 ) {
        printf("\t=> not enough memory to perform the write\n");
        return -1; /* we don't have enough memory to perform the write */
    }

    /* perform the write */
    strncpy( (char*)(&file->data.buf[offset]), (const char*)buf, (size_t)len );

    /* adjust file->data.len */
    file->data.len = end_offset;

    printf("\t=> wrote %d bytes\n", len);
    return len;
}

#endif //SQLITE_OS_OTHER