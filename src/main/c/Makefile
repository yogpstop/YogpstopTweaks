CC=gcc
CFLAGS=-c -Wall -Werror -O3 -march=native -pipe -flto
LD=gcc
LDFLAGS=-s

SRCS=$(wildcard *.c)

ifneq ($(strip $(DEBUG)),)
CFLAGS+=-DDEBUG=1
endif

OBJS=$(SRCS:%.c=%.o)
LIBS=-lz

all: backup
backup: $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)
.c.o:
	$(CC) -o $@ $(CFLAGS) $<
clean:
	rm -f $(OBJS) backup backup.exe
.PHONY: clean
