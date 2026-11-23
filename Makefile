CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -pthread

all: vm

vm: vm.c
	$(CC) $(CFLAGS) -o vm vm.c $(LDFLAGS)

clean:
	rm -f vm correct.txt

.PHONY: all clean
