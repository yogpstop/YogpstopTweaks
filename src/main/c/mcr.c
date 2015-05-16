#include <stdlib.h>
#include "main.h"
#define MCRZL_INF 1
#include "mcrzl.h"
#undef MCRZL_INF
#include "mcrzl.h"

#define U8P(p, o) (*((uint8_t*)(((void*)(p)) + (o))))
#define rU32P(p, o) (*((uint32_t*)(((void*)(p)) + (o))))
//FIXME endian
#define U8_24(p, o) ((U8P(p, o) << 16) | (U8P(p, o + 1) << 8) \
		| (U8P(p, o + 2)))
#define U8_32(p, o) ((U8P(p, o) << 24) | (U8P(p, o + 1) << 16) \
		| (U8P(p, o + 2) << 8) | (U8P(p, o + 3)))
int get_mcr(void *in, const size_t ilen, int n,
		uint8_t *ct, uint32_t *ts, void **out, size_t *olen) {
	uint16_t i;
	size_t p;
	while (1) {
		i = n * 4;
		p = U8_24(in, i) * 4096;
		*ts = rU32P(in, i + 4096);
		if (p >= 8192) break;
		if (n >= 1023) {*out = NULL; *olen = 0; return -1; }
		n++;
	}
	dbgprintf("get_mcr %04X %"PFZ"u\n", i, p);
	*ct = U8P(in, p + 4);
	if (*ct & DT_ZLIB)
		*out = zlib_inf(in + p + 5, U8_32(in, p) - 1, olen);
	else {
		*out = NULL; *olen = 0; }//FIXME extract gzip
	dbgprintf("GET_MCR\n");
	return n;
}
