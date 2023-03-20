CC=gcc
CFLAGS= -Wall -D_FILE_OFFSET_BITS=64 -lfuse
# CFLAGS= -Wall `pkg-config fuse3 --cflags --libs`
# DEPS = russ_test.h
OBJ = russ_test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

out: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean
clean: 
	rm -f *.o *~ out 