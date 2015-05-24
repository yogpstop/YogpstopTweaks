#include <stdlib.h>
#include <string.h>
#include "main.h"

static int dec_xz_read(void *a1, void *a2, unsigned a3) {
	return xz_d_read(a1, a2, a3);
}
static int dec_gz_read(void *a1, void *a2, unsigned a3) {
	return gzread(a1, a2, a3);
}
st_decomp dec_init(void *in, int xz) {
	st_decomp ret = malloc(sizeof(sst_decomp));
	memset(ret, 0, sizeof(sst_decomp));
	ret->xz = xz;
	if (xz) {
		ret->in = in;
		ret->read = dec_xz_read;
	} else {
		ret->in = gzopen(in ,"rb");
		gzbuffer(ret->in, 1024 * 1024 * 16);//16MB buffer
		ret->read = dec_gz_read;
	}
	return ret;
}
void dec_final(st_decomp obj) {
	if (obj->out) free(obj->out);
	if (obj->name) free(obj->name);
	if (!obj->xz) gzclose(obj->in);
	free(obj);
}
// must be sizeof(size_t) * 8 < UINT8_MAX
static size_t read_len(st_decomp obj) {
	uint8_t len = 0, tmp;
	size_t ret = 0;
	while (!((ret >> (len * 7)) & 1)) {
		len += obj->read(obj->in, &tmp, 1);// must return 1
		ret = (ret << 8) | tmp;
	}
	return ret & ~(1 << (len * 7));
}
int dec_do(st_decomp obj) {
	uint8_t tmp;
	if (!obj->read(obj->in, &tmp, 1)) return 0;
	obj->type = tmp;
	if (obj->type & DT_NEW) {
		size_t nlen = read_len(obj);
		if (obj->name) free(obj->name);
		obj->name = malloc(nlen + 1);
		obj->read(obj->in, obj->name, nlen);
		obj->name[nlen] = 0;
		obj->type &= ~DT_NEW;
	}
	if (obj->type & DT_MCR) {
		obj->read(obj->in, &obj->ts, 4);// must return 4
		obj->read(obj->in, &tmp, 1);// must return 1
		obj->type |= tmp << 8;
	}
	obj->len = read_len(obj);
	if (obj->out) free(obj->out);
	obj->out = malloc(obj->len);
	obj->read(obj->in, obj->out, obj->len);
	return 1;
}
