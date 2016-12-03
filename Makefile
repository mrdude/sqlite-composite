CFLAGS=-DSQLITE_OS_OTHER=1
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
EXE=sqlite

all:
	$(CC) $(CFLAGS) -o $(EXE) $(SRC)

clean:
	rm -f $(EXE)