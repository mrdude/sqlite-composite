
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