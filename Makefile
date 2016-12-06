CFLAGS+= -DSQLITE_OS_OTHER=1
CFLAGS+= -DSQLITE_COS_PROFILE_VFS=1
CFLAGS+= -DDSQLITE_COS_PROFILE_MUTEX=1
CFLAGS+= -DSQLITE_COS_PROFILE_MEMORY=0
CFLAGS+= -g

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
HDR=$(wildcard *.h)
EXE=sqlite

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

$(OBJ): $(@:.o=.c) $(HDR)
	$(CC) $(CFLAGS) -c -o $@ $(@:.o=.c)

clean:
	rm -f $(EXE) $(OBJ)