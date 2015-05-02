CC=gcc
CFLAGS=-Wall -Werror -O3 -march=native -pipe -flto
LD=gcc
LDFLAGS=-s

SRCS=$(wildcard *.c)
OBJS=$(SRCS:%.c=%.o)
DEPS=$(SRCS:%.c=%.d)
LIBS=-lz

ifeq ($(OS),Windows_NT)
EXEEXT=.exe
endif

ifneq ($(strip $(DEBUG)),)
CFLAGS+=-DDEBUG=1
endif

all: backup$(EXEEXT)

-include $(DEPS)

backup$(EXEEXT): $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)
.c.o:
	$(CC) -c -MMD -MP -o $@ $(CFLAGS) $<
clean:
	rm -f $(OBJS) $(DEPS) backup$(EXEEXT)
.PHONY: clean
