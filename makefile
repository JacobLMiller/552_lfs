CC=gcc
CFLAGS= -Wall -D_FILE_OFFSET_BITS=64 -lfuse
# CFLAGS= -Wall `pkg-config fuse3 --cflags --libs`
DEPS = global.h types.h log.h lfs.h
OBJ_MKLFS = flash.o mklfs.o
OBJ_LFS = buggy_crc.o flash.o lfs.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lfs: $(OBJ_LFS)
	$(CC) -o $@ $^ $(CFLAGS)

mklfs: $(OBJ_MKLFS)
	$(CC) -o $@ $^ $(CFLAGS)

russtest: flash.o russ_test.o 
	$(CC) -o $@ $^ $(CFLAGS)




.PHONY: clean
clean: 
	rm -f *.o *~ out russtest lfs mklfs myfile
	