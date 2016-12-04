
#ifndef SQLITE_COS_OS_COMPOSITE_H
#define SQLITE_COS_OS_COMPOSITE_H

#ifndef SQLITE_COS_PROFILE_VFS
#define SQLITE_COS_PROFILE_VFS 0
#endif

#ifndef SQLITE_COS_PROFILE_MUTEX
#define SQLITE_COS_PROFILE_MUTEX 0
#endif

#ifndef SQLITE_COS_PROFILE_MEMORY
#define SQLITE_COS_PROFILE_MEMORY 0
#endif

#if SQLITE_COS_PROFILE_VFS || SQLITE_COS_PROFILE_MUTEX || SQLITE_COS_PROFILE_MEMORY
#define CTRACE_STRING_DEF(x) const int LEN = x ; \
    char _ch[ LEN+1 ]; \
    do { \
        int i = 0; \
        for( i = 0; i < LEN+1; i++ ) \
            _ch[i] = 0; \
    } while(0)

#define CTRACE_APPEND(...) do { snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch), __VA_ARGS__ ); } while(0)
#define CTRACE_PRINT(...) do { printf("%s\n", &_ch[0]); } while(0)

#define APPEND_ERR_CODE(x) do { \
    switch( x ) { \
        case SQLITE_OK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_OK"); break; \
        case SQLITE_ERROR: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_ERROR"); break; \
        case SQLITE_INTERNAL: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_INTERNAL"); break; \
        case SQLITE_PERM: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_PERM"); break; \
        case SQLITE_ABORT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_ABORT"); break; \
        case SQLITE_BUSY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_BUSY"); break; \
        case SQLITE_LOCKED: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_LOCKED"); break; \
        case SQLITE_NOMEM: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOMEM"); break; \
        case SQLITE_READONLY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_READONLY"); break; \
        case SQLITE_INTERRUPT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_INTERRUPT"); break; \
        case SQLITE_IOERR: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR"); break; \
        case SQLITE_CORRUPT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CORRUPT"); break; \
        case SQLITE_NOTFOUND: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOTFOUND"); break; \
        case SQLITE_FULL: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_FULL"); break; \
        case SQLITE_CANTOPEN: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CANTOPEN"); break; \
        case SQLITE_PROTOCOL: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_PROTOCOL"); break; \
        case SQLITE_EMPTY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_EMPTY"); break; \
        case SQLITE_SCHEMA: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_SCHEMA"); break; \
        case SQLITE_TOOBIG: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_TOOBIG"); break; \
        case SQLITE_CONSTRAINT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT"); break; \
        case SQLITE_MISMATCH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_MISMATCH"); break; \
        case SQLITE_MISUSE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_MISUSE"); break; \
        case SQLITE_NOLFS: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOLFS"); break; \
        case SQLITE_AUTH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_AUTH"); break; \
        case SQLITE_FORMAT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_FORMAT"); break; \
        case SQLITE_RANGE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_RANGE"); break; \
        case SQLITE_NOTADB: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOTADB"); break; \
        case SQLITE_NOTICE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOTICE"); break; \
        case SQLITE_WARNING: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_WARNING"); break; \
        case SQLITE_ROW: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_ROW"); break; \
        case SQLITE_DONE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_DONE"); break; \
        case SQLITE_IOERR_READ: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_"); break; \
        case SQLITE_IOERR_SHORT_READ: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SHORT_READ"); break; \
        case SQLITE_IOERR_WRITE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_WRITE"); break; \
        case SQLITE_IOERR_FSYNC: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_FSYNC"); break; \
        case SQLITE_IOERR_DIR_FSYNC: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_DIR_FSYNC"); break; \
        case SQLITE_IOERR_TRUNCATE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_TRUNCATE"); break; \
        case SQLITE_IOERR_FSTAT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_FSTAT"); break; \
        case SQLITE_IOERR_UNLOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_UNLOCK"); break; \
        case SQLITE_IOERR_RDLOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_RDLOCK"); break; \
        case SQLITE_IOERR_DELETE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_DELETE"); break; \
        case SQLITE_IOERR_BLOCKED: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_BLOCKED"); break; \
        case SQLITE_IOERR_NOMEM: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_NOMEM"); break; \
        case SQLITE_IOERR_ACCESS: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_ACCESS"); break; \
        case SQLITE_IOERR_CHECKRESERVEDLOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_CHECKRESERVEDLOCK"); break; \
        case SQLITE_IOERR_LOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_LOCK"); break; \
        case SQLITE_IOERR_CLOSE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_CLOSE"); break; \
        case SQLITE_IOERR_DIR_CLOSE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_DIR_CLOSE"); break; \
        case SQLITE_IOERR_SHMOPEN: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SHMOPEN"); break; \
        case SQLITE_IOERR_SHMSIZE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SHMSIZE"); break; \
        case SQLITE_IOERR_SHMLOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SHMLOCK"); break; \
        case SQLITE_IOERR_SHMMAP: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SHMMAP"); break; \
        case SQLITE_IOERR_SEEK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_SEEK"); break; \
        case SQLITE_IOERR_DELETE_NOENT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_DELETE_NOENT"); break; \
        case SQLITE_IOERR_MMAP: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_MMAP"); break; \
        case SQLITE_IOERR_GETTEMPPATH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_GETTEMPPATH"); break; \
        case SQLITE_IOERR_CONVPATH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_CONVPATH"); break; \
        case SQLITE_IOERR_VNODE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_VNODE"); break; \
        case SQLITE_IOERR_AUTH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_IOERR_AUTH"); break; \
        case SQLITE_LOCKED_SHAREDCACHE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_LOCKED_SHAREDCACHE"); break; \
        case SQLITE_BUSY_RECOVERY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_BUSY_RECOVERY"); break; \
        case SQLITE_BUSY_SNAPSHOT: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_BUSY_SNAPSHOT"); break; \
        case SQLITE_CANTOPEN_NOTEMPDIR: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CANTOPEN_NOTEMPDIR"); break; \
        case SQLITE_CANTOPEN_ISDIR: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CANTOPEN_ISDIR"); break; \
        case SQLITE_CANTOPEN_FULLPATH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CANTOPEN_FULLPATH"); break; \
        case SQLITE_CANTOPEN_CONVPATH: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CANTOPEN_CONVPATH"); break; \
        case SQLITE_CORRUPT_VTAB: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CORRUPT_VTAB"); break; \
        case SQLITE_READONLY_RECOVERY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_READONLY_RECOVERY"); break; \
        case SQLITE_READONLY_CANTLOCK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_READONLY_CANTLOCK"); break; \
        case SQLITE_READONLY_ROLLBACK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_READONLY_ROLLBACK"); break; \
        case SQLITE_READONLY_DBMOVED: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_READONLY_DBMOVED"); break; \
        case SQLITE_ABORT_ROLLBACK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_ABORT_ROLLBACK"); break; \
        case SQLITE_CONSTRAINT_CHECK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_CHECK"); break; \
        case SQLITE_CONSTRAINT_COMMITHOOK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_COMMITHOOK"); break; \
        case SQLITE_CONSTRAINT_FOREIGNKEY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_FOREIGNKEY"); break; \
        case SQLITE_CONSTRAINT_FUNCTION: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_FUNCTION"); break; \
        case SQLITE_CONSTRAINT_NOTNULL: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_NOTNULL"); break; \
        case SQLITE_CONSTRAINT_PRIMARYKEY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_PRIMARYKEY"); break; \
        case SQLITE_CONSTRAINT_TRIGGER: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_TRIGGER"); break; \
        case SQLITE_CONSTRAINT_UNIQUE: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_UNIQUE"); break; \
        case SQLITE_CONSTRAINT_VTAB: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_VTAB"); break; \
        case SQLITE_CONSTRAINT_ROWID: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_CONSTRAINT_ROWID"); break; \
        case SQLITE_NOTICE_RECOVER_WAL: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOTICE_RECOVER_WAL"); break; \
        case SQLITE_NOTICE_RECOVER_ROLLBACK: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_NOTICE_RECOVER_ROLLBACK"); break; \
        case SQLITE_WARNING_AUTOINDEX: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_WARNING_AUTOINDEX"); break; \
        case SQLITE_AUTH_USER: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_AUTH_USER"); break; \
        case SQLITE_OK_LOAD_PERMANENTLY: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"SQLITE_OK_LOAD_PERMANENTLY"); break; \
        default: snprintf(&_ch[ strlen(_ch) ], LEN - strlen(_ch),"Unknown(%d)", x ); break; \
    } \
    } while(0)
#endif

/* API structs */
extern struct sqlite3_io_methods composite_io_methods;
extern struct composite_vfs_data composite_vfs_app_data;

/* cFile */
struct cFile {
    struct sqlite3_io_methods* composite_io_methods;
    const char* zName;
    int fd;
    int closed;
    int deleteOnClose;
};

struct composite_vfs_data {
    int random_fd;
};

/* sqlite_io function prototypes */
int cClose(sqlite3_file* file);
int cRead(sqlite3_file* file, void* buf, int iAmt, sqlite3_int64 iOfst);
int cWrite(sqlite3_file* file, const void* buf, int iAmt, sqlite3_int64 iOfst);
int cTruncate(sqlite3_file* file, sqlite3_int64 size);
int cSync(sqlite3_file* file, int flags);
int cFileSize(sqlite3_file* file, sqlite3_int64 *pSize);
int cLock(sqlite3_file* file, int i);
int cUnlock(sqlite3_file* file, int i);
int cCheckReservedLock(sqlite3_file* file, int *pResOut);
int cFileControl(sqlite3_file* file, int op, void *pArg);
int cSectorSize(sqlite3_file* file);
int cDeviceCharacteristics(sqlite3_file* file);
int cShmMap(sqlite3_file* file, int iPg, int pgsz, int i, void volatile** v);
int cShmLock(sqlite3_file* file, int offset, int n, int flags);
void cShmBarrier(sqlite3_file* file);
int cShmUnmap(sqlite3_file* file, int deleteFlag);
int cFetch(sqlite3_file* file, sqlite3_int64 iOfst, int iAmt, void **pp);
int cUnfetch(sqlite3_file* file, sqlite3_int64 iOfst, void *p);

/* sqlite_vfs function prototypes */
int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags);
int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir);
int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut);
int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut);
int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut);
int cSleep(sqlite3_vfs* vfs, int microseconds);
int cGetLastError(sqlite3_vfs* vfs, int i, char *ch);
int cCurrentTime(sqlite3_vfs* vfs, double* time);
int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time);

/* sqlite_mutex function prototypes */
int cMutexInit(void);
int cMutexEnd(void);
sqlite3_mutex* cMutexAlloc(int mutexType);
void cMutexFree(sqlite3_mutex *mutex);
void cMutexEnter(sqlite3_mutex *mutex);
int cMutexTry(sqlite3_mutex *mutex);
void cMutexLeave(sqlite3_mutex *mutex);
int cMutexHeld(sqlite3_mutex *mutex);
int cMutexNotheld(sqlite3_mutex *mutex);

/* sqlite_mem function prototypes */
void *cMemMalloc(int sz);         /* Memory allocation function */
void cMemFree(void* mem);          /* Free a prior allocation */
void *cMemRealloc(void* mem, int newSize);  /* Resize an allocation */
int cMemSize(void* mem);           /* Return the size of an allocation */
int cMemRoundup(int sz);          /* Round up request size to allocation size */
int cMemInit(void* pAppData);           /* Initialize the memory allocator */
void cMemShutdown(void* pAppData);      /* Deinitialize the memory allocator */

#endif