#include <stdlib.h>
#include <string.h>
#include "main.h"

void comp_final(st_compress obj) {
	if (obj->prev) free(obj->prev);
	gzclose(obj->f_out);
	free(obj);
}
st_compress comp_init(char *out, int speed) {
	st_compress ret = malloc(sizeof(sst_compress));
	memset(ret, 0, sizeof(sst_compress));
	ret->f_out = gzopen(out, "wb");
	gzbuffer(ret->f_out, 1024 * 1024 * 16);//16MB buffer
	gzsetparams(ret->f_out,
			speed ? Z_BEST_SPEED : Z_BEST_COMPRESSION, Z_DEFAULT_STRATEGY);
	return ret;
}
static size_t st_len(size_t len) {
	size_t ret = 0, tmp = len;
	do {
		tmp = tmp >> 7;
		ret++;
	} while (tmp);
	return ret;
}
static void st_write(size_t len, void **cur) {
	size_t tmp = len, b = st_len(len);
	tmp |= 1 << (b * 7);
	while (b--)
		*((uint8_t*)(*cur)++) = (tmp >> (b * 8)) & 0xFF;
}
void comp_do(st_compress obj, uint16_t type, char *name,
		uint32_t ts, size_t len, void *data) {
	dbgprintf("comp_do %04X %08X %"PFZ"u %16p %s\n", type, ts, len, data, name);
	int sname = obj->prev && !strcmp(name, obj->prev);
	size_t in_remain = 1 + (sname ? 0 : st_len(strlen(name)) + strlen(name))
			+ (DT_IS(type, DT_MCR) ? 5 : 0) + st_len(len) + len;
	void * const in_buf = malloc(in_remain);
	void *cur = in_buf;
	*((uint8_t*)cur++) = (uint8_t) (type | (sname ? 0 : DT_NEW));
	if (!sname) {
		size_t nlen = strlen(name);
		if (obj->prev) free(obj->prev);
		obj->prev = malloc(nlen + 1);
		memcpy(obj->prev, name, nlen);
		obj->prev[nlen] = 0;
		st_write(nlen, &cur);
		memcpy(cur, name, nlen);
		cur += nlen;
	}
	if (DT_IS(type, DT_MCR)) {
		memcpy(cur, &ts, 4);
		cur += 4;
		*((uint8_t*)cur++) = (uint8_t) (type >> 8);
	}
	st_write(len, &cur);
	memcpy(cur, data, len);
	cur = in_buf;
	size_t tmp = 0;
	while ((tmp = gzwrite(obj->f_out, cur, in_remain))) {
		cur += tmp; in_remain -= tmp; }
	free(in_buf);
	dbgprintf("COMP_DO\n");
}
