
#ifndef SQLITE_COS_OS_COMPOSITE_H
#define SQLITE_COS_OS_COMPOSITE_H

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

#endif