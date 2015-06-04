CC:=gcc
CFLAGS:=-Wall -Werror -O3 -march=native -pipe -flto
LD:=gcc
LDFLAGS:=-s

SRCS:=$(wildcard *.c bsdiff/*.c)
OBJS:=$(SRCS:%.c=%.o)
DEPS:=$(SRCS:%.c=%.d)
LIBS:=-lz -lm $(shell pkg-config --libs liblzma)
CCFLAGS:=$(CFLAGS) $(shell pkg-config --cflags liblzma)
LLDFLAGS:=$(LDFLAGS)

ifeq ($(SYS), mingw)
EXT:=.exe
LIBS+=-lws2_32
LLDFLAGS+=-static -mwindows
endif

ifneq ($(strip $(DEBUG)),)
CCFLAGS+=-DDEBUG=1
endif

all: backup$(EXT)

-include $(DEPS)

backup$(EXT): $(OBJS)
	$(LD) -o $@ $(LLDFLAGS) $^ $(LIBS)
.c.o:
	$(CC) -c -MMD -MP -o $@ $(CCFLAGS) $<
clean:
	rm -f $(OBJS) $(DEPS) backup$(EXT)
.PHONY: clean
