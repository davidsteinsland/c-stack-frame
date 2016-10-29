CC=gcc
CCOPTS=-Wall -m32 -g -O0
LDOPTS=-arch i386

.PHONY: all clean

all: main

main: main.o
	$(CC) $(CCOPTS) $(LDOPTS) -o $@ $^

clean:
	-$(RM) *.o

%.o: %.c
	$(CC) $(CCOPTS) -c -o $@ $<
