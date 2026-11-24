CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -pthread
FRAMES ?= 128

all: vm

vm: vm.c
	$(CC) $(CFLAGS) -DFRAMES=$(FRAMES) -o vm vm.c $(LDFLAGS)

clean:
	rm -f vm correct.txt

.PHONY: all clean
