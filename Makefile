CFLAGS+= -DSQLITE_OS_OTHER=1
CFLAGS+= -DSQLITE_COS_PROFILE_VFS=1
CFLAGS+= -DDSQLITE_COS_PROFILE_MUTEX=0
CFLAGS+= -DSQLITE_COS_PROFILE_MEMORY=0
CFLAGS+= -DSQLITE_MAX_MMAP_SIZE=0
CFLAGS+= -DSQLITE_THREADSAFE=0
CFLAGS+= -g

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
HDR=$(wildcard *.h)
EXE=sqlite
COS_SRC_INPUT=os_composite_mem.c os_composite_mutex.c os_composite_inmemfs.c os_composite_vfs.c os_composite.c
COS_SRC_AMALGAMATION=composite_sqlite.c

.PHONY: all composite clean

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

$(OBJ): $(@:.o=.c) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

composite:
	cat $(COS_SRC_INPUT) > $(COS_SRC_AMALGAMATION)
	$(CC) $(CFLAGS) -c -o $(EXE) $(COS_SRC_AMALGAMATION) shell.c sqlite3.c

clean:
	rm -f $(EXE) $(OBJ) $(COS_SRC_AMALGAMATION)