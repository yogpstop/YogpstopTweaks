CC=gcc
CFLAGS=-Wall -Werror -O3 -march=native -pipe -flto
LD=gcc
LDFLAGS=-s -static

SRCS=$(wildcard *.c bsdiff/*.c)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)
LIBS=-lz $(shell pkg-config --libs liblzma)
CCFLAGS=$(CFLAGS) $(shell pkg-config --cflags liblzma)

ifeq ($(OS),Windows_NT)
EXEEXT=.exe
endif

ifneq ($(strip $(DEBUG)),)
CCFLAGS+=-DDEBUG=1
endif

all: backup$(EXEEXT)

-include $(DEPS)

backup$(EXEEXT): $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)
.c.o:
	$(CC) -c -MMD -MP -o $@ $(CCFLAGS) $<
clean:
	rm -f $(OBJS) $(DEPS) backup$(EXEEXT)
.PHONY: clean
