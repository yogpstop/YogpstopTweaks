CC=gcc
CFLAGS=-c -Wall -Werror -O3 -march=native -pipe -flto
LD=gcc
LDFLAGS=-s

SRCS=$(wildcard *.c)

ifeq ($(strip $(NSZIP)),)
SZDIR=/c/Users/yogpstop/Downloads/lzma938/C
SRCS+=$(addprefix $(SZDIR)/,LzmaDec.c LzmaEnc.c MtCoder.c LzFindMt.c LzFind.c Threads.c)
CFLAGS+=-I$(SZDIR)
else
CFLAGS+=-DNSZIP=1
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
