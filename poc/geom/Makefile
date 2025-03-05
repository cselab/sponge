.POSIX:
.SUFFIX:
.SUFFIX: .c

CC = c99
CFLAGS = -O2

all: stl
.c:
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -f stl

