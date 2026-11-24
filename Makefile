CC      = gcc
CFLAGS  = -Wall -Wextra -g
LDFLAGS = -pthread
FRAMES ?= 128
TLB    ?= 1
THREADS ?= 1
all: vm

vm: vm.c
	$(CC) $(CFLAGS) -DFRAMES=$(FRAMES) -DUSA_TLB=$(TLB) -DUSA_THREADS=$(THREADS) -o vm vm.c $(LDFLAGS)

clean:
	rm -f vm correct.txt

.PHONY: all clean
