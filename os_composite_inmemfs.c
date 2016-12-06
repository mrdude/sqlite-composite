/* Contains code for an in-memory filesystem implementation */

#include "sqlite3.h"
#if SQLITE_OS_OTHER

/* TODO: when I port this to composite, I won't have access to these headers. */
#include <string.h> /* for strncpy() and strncmp() */
#include <inttypes.h> /* for PRIu64 */
#include <stdio.h> /* for debugging printf()'s in fs_write and fs_read */

#include "os_composite.h"

#define INITIAL_BUF_DATA_SIZE (FS_SECTOR_SIZE*2)

/* the maximum possible size of a file; the largest value that can be represented with a signed 64-bit int
 * on 32-bit systems, the actual minimum will be much lower.
 */
#define MAX_FILE_LEN ( (int64_t)(1L<<(sizeof(int64_t)-1)) )

#define MIN(type, a, b) ( ((type)(a)) < ((type)(b)) ? (type)(a) : (type)(b) )

static struct fs_file* _fs_file_list = 0;

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

static char* _fs_copystring(const char* str, int n) {
    /* get the length of the string */
    int len;
    for( len = 0; str[len] != '\0' && len < n; len++ ) {}
    printf("_fs_copystring(str = %s) | len = %d\n", str, len);

    /* malloc() some memory for our copy */
    char* new_str = _FS_MALLOC(len+1);
    if( new_str == 0 ) {
        return 0;
    }

    /* perform the copy */
    int i = 0;
    for( i = 0; i < len; i++ ) {
        new_str[i] = str[i];
    }
    new_str[len] = 0;

    return new_str;
}

/* adds the given file to _fs_file_list */
static void _fs_file_link(struct fs_file* file) {
    file->next = _fs_file_list->next;
    _fs_file_list->next = file;
}

/* removes the given file from _fs_file_list */
static void _fs_file_unlink(struct fs_file* file) {
    struct fs_file* prev = 0;
    struct fs_file* next = _fs_file_list->next;

    while( next != 0 ) {
        if( next == file ) {
            if( prev ) prev->next = next->next;
            file->next = 0;
            break;
        }

        prev = next;
        next = next->next;
    }
}

/* searches _fs_file_list for the file with the given name, or 0 if it doesn't exist */
static struct fs_file* _fs_find_file(sqlite3_vfs* vfs, const char* zName) {
    struct fs_file* file;
    for( file = _fs_file_list; file != 0; file = file->next ) {
        if( strncmp(file->zName, zName, MAX_PATHNAME) == 0 ) {
            printf("\t_fs_find_file(zName = %s) => file = %p\n", zName, file);
            return file;
        }
    }
    printf("\t_fs_find_file(zName = %s) => file = 0\n", zName);
    return 0;
}

static struct fs_file* _fs_file_alloc(sqlite3_vfs* vfs, const char *zName) {
    printf("\t_fs_file_alloc(zName = %s)\n", zName);
    struct composite_vfs_data* cVfs = (struct composite_vfs_data*)(vfs->pAppData);
    struct fs_file* file = _FS_MALLOC( sizeof(struct fs_file) );
    if( file == 0 )
        return 0;
    
    char* buf = _FS_MALLOC( INITIAL_BUF_DATA_SIZE );
    if( buf == 0 ) {
        _FS_FREE(file);
        return 0;
    }

    char* zNameCopy = _fs_copystring(zName, MAX_PATHNAME);
    if( zNameCopy == 0 ) {
        _FS_FREE(file);
        _FS_FREE(buf);
        return 0;
    }
    
    file->cVfs = cVfs;
    file->next = 0;
    file->zName = zNameCopy;
    file->data.buf = buf;
    file->data.len = 0;
    file->ref = 0;
    file->deleteOnClose = 0;

    _fs_file_link(file);

    return file;
}

/* free the file and all of its blocks */
static void _fs_file_free(struct fs_file* file) {
    _FS_FREE( (char*)file->zName );
    _FS_FREE( file->data.buf );
    _FS_FREE( file );

    file->zName = 0;
    file->data.buf = 0;
}

/* makes sure that there is sz bytes of space in file's data buffer
 * returns 1 on success, 0 on failure
 */
static int _fs_data_ensure_capacity(struct fs_file* file, sqlite3_int64 sz) {
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
    _fs_file_list = _FS_MALLOC( sizeof(struct fs_file) );
}

void fs_deinit() {
    /* free all files from memory */
    struct fs_file* file = _fs_file_list->next;
    for( ; file != 0; file = file->next ) {
        void* next = file->next;
        _fs_file_free(file);
        file = next;
    }

    /* clear the file list */
    _FS_FREE( _fs_file_list );
    _fs_file_list = 0;
}

struct fs_file* fs_open(sqlite3_vfs* vfs, const char* zName) {
    //TODO make this atomic
    //atomic {
    struct fs_file* file = _fs_find_file(vfs, zName);
    if( file == 0 ) {
        file = _fs_file_alloc(vfs, zName);
    }
    //}
    if( file == 0 ) {
        printf("\tfs_open(zName = %s) => 0\n", zName);
        return 0;
    }

    file->ref++;
    printf("\tfs_open(zName = %s) => file = %p\n", zName, file);
    return file;
}

void fs_close(struct fs_file* file) {
    //TODO make this threadsafe
    file->ref--;
    if( file->ref == 0 && file->deleteOnClose ) { /* have we been waiting to delete this file? */
        _fs_file_unlink(file); //unlink the file from the list of files
        _fs_file_free(file); //free the memory the file used
    }
}

/* returns the number of bytes read, or -1 if an error occurred. short reads are allowed. */
int fs_read(struct fs_file* file, sqlite3_int64 offset, int len, void* buf) {
    //TODO check locks

    /* perform sanity checks on offset and len */
    if( offset < 0 || len < 0 ) {
        return -1;
    }
    
    /* determine the number of bytes to read */
    sqlite3_int64 end_offset = offset + (sqlite3_int64)len;
    if( end_offset > file->data.len ) end_offset = file->data.len;
    int bytes_read = (int)(end_offset - offset);

    /* copy the bytes into the buffer */
    if( bytes_read > 0 ) {
        strncpy( (char*)buf, (const char*)(&file->data.buf[offset]), (size_t)bytes_read);
    }

    printf("\tfs_read(file = %s, offset = %" PRIu64 ", len = %d, buf = <...>) => read %d bytes\n",
        file->zName,
        offset,
        len,
        bytes_read);
    return bytes_read;
}

/* returns the number of bytes written, or -1 if an error occurred. partial writes are not allowed. */
int fs_write(struct fs_file* file, sqlite3_int64 offset, int len, const void* buf) {
    //TODO check locks -- this should occur atomically

    /* perform sanity checks on offset and len */
    if( offset < 0 || len < 0 ) {
        return -1;
    }

    /* ensure that our buffer is large enough to perform the write */
    sqlite3_int64 end_offset = offset + (sqlite3_int64)len;
    if( _fs_data_ensure_capacity(file, end_offset) == 0 ) {
        return -1; /* we don't have enough memory to perform the write */
    }

    /* perform the write */
    strncpy( (char*)(&file->data.buf[offset]), (const char*)buf, (size_t)len );

    /* adjust file->data.len */
    file->data.len = end_offset;

    printf("\tfs_write(file = %s, offset = %" PRIu64 ", len = %d, buf = <...>) => wrote %d bytes\n",
        file->zName,
        offset,
        len,
        len);
    return len;
}

/* returns 1 on success, 0 on failure */
int fs_truncate(struct fs_file* file, sqlite3_int64 size) {
    if( size < file->data.len ) {
        file->data.len = size; //TODO reclaim this memory
    }
    return 1;
}

/* returns 1 if the given file exists, 0 if it doesn't */
int fs_exists(sqlite3_vfs* vfs, const char *zName) {
    struct fs_file* file = _fs_find_file(vfs, zName);
    return (file != 0);
}

/* returns 1 on success, 0 on failure */
int fs_delete(sqlite3_vfs* vfs, const char *zName) {
    struct fs_file* file = _fs_find_file(vfs, zName);
    if( file == 0 ) { /* the file doesn't exist */
        return 1;
    }

    if( file->ref == 0 ) { /* no one has this file open currently */
        _fs_file_unlink(file);
        _fs_file_free(file);
    } else { /* the file is open somewhere */
        file->deleteOnClose = 1; /* when this file is closed, it will be deleted */
    }
}

#endif //SQLITE_OS_OTHER