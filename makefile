CC=gcc
CFLAGS= -Wall -D_FILE_OFFSET_BITS=64 -lfuse
# CFLAGS= -Wall `pkg-config fuse3 --cflags --libs`
DEPS = types.h
OBJ = flash.o mklfs.o
OBJ2 = flash.o lfs.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lfs: $(OBJ2)
	$(CC) -o $@ $^ $(CFLAGS)

mklfs: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)




.PHONY: clean
clean: 
	rm -f *.o *~ out 
	