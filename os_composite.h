#ifndef SQLITE_OS_COMPOSITE_H
#define SQLITE_OS_COMPOSITE_H

struct sqlite3_io_methods;
struct sqlite3_vfs;
struct sqlite3_int64;

/* API structs */
extern static struct sqlite3_io_methods composite_io_methods;
extern static sqlite3_vfs composite_vfs;

/* cFile */
struct cFile {
    struct sqlite3_io_methods* composite_io_methods;
    const char* zName;
};

/* function prototypes */
static int cClose(sqlite3_file* file);
static int cRead(sqlite3_file* file, void* buf, int iAmt, sqlite3_int64 iOfst);
static int cWrite(sqlite3_file* file, const void* buf, int iAmt, sqlite3_int64 iOfst);
static int cTruncate(sqlite3_file* file, sqlite3_int64 size);
static int cSync(sqlite3_file* file, int flags);
static int cFileSize(sqlite3_file* file, sqlite3_int64 *pSize);
static int cLock(sqlite3_file* file, int i);
static int cUnlock(sqlite3_file* file, int i);
static int cCheckReservedLock(sqlite3_file* file, int *pResOut);
static int cFileControl(sqlite3_file* file, int op, void *pArg);
static int cSectorSize(sqlite3_file* file);
static int cDeviceCharacteristics(sqlite3_file* file);
static int cShmMap(sqlite3_file* file, int iPg, int pgsz, int i, void volatile** v);
static int cShmLock(sqlite3_file* file, int offset, int n, int flags);
static void cShmBarrier(sqlite3_file* file);
static int cShmUnmap(sqlite3_file* file, int deleteFlag);
static int cFetch(sqlite3_file* file, sqlite3_int64 iOfst, int iAmt, void **pp);
static int cUnfetch(sqlite3_file* file, sqlite3_int64 iOfst, void *p);

static int cOpen(sqlite3_vfs* vfs, const char *zName, sqlite3_file* baseFile, int flags, int *pOutFlags);
static int cDelete(sqlite3_vfs* vfs, const char *zName, int syncDir);
static int cAccess(sqlite3_vfs* vfs, const char *zName, int flags, int *pResOut);
static int cFullPathname(sqlite3_vfs* vfs, const char *zName, int nOut, char *zOut);
static int cRandomness(sqlite3_vfs* vfs, int nByte, char *zOut);
static int cSleep(sqlite3_vfs* vfs, int microseconds);
static int cGetLastError(sqlite3_vfs* vfs, int i, char *ch);
static int cCurrentTime(sqlite3_vfs* vfs, double* time);
static int cCurrentTimeInt64(sqlite3_vfs* vfs, sqlite3_int64* time);

#endif