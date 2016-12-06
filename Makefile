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
COS_OBJ=sqlite.o

.PHONY: all composite clean

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

$(OBJ): $(@:.o=.c) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

composite:
	$(CC) $(CFLAGS) -c -o $(COS_OBJ) $(SRC)

clean:
	rm -f $(EXE) $(OBJ)