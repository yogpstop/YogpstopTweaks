CFLAGS=-Wall -Werror -O3 -s -march=native -pipe -flto
SZDIR=/c/Users/yogpstop/Downloads/lzma938/C
CC=gcc
all:
	$(CC) $(CFLAGS) -I$(SZDIR) *.c $(SZDIR)/{LzmaDec.c,LzmaEnc.c,MtCoder.c,LzFindMt.c,LzFind.c,Threads.c} -lz
