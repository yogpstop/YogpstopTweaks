#include <stdlib.h>
#include <string.h>
#include "main.h"

st_xz_d xz_d_init(char *inn) {//open,buffer
	st_xz_d ret = malloc(sizeof(sst_xz_d));
	memset(ret, 0, sizeof(sst_xz_d));
	if (LZMA_OK != lzma_stream_decoder(&ret->ls, UINT64_MAX, 0)) {
		free(ret); return NULL;
	}
	ret->in = fopen(inn, "rb");
	ret->inbl = 1024 * 1024 * 16;
	ret->inb = malloc(ret->inbl);
	if (!ret->in || !ret->inb) {
		lzma_end(&ret->ls);
		if (ret->in) fclose(ret->in);
		if (ret->inb) free(ret->inb);
		free(ret);
		return NULL;
	}
	return ret;
}

void xz_d_final(st_xz_d obj) {//close
	free(obj->inb);
	fclose(obj->in);
	lzma_end(&obj->ls);
	free(obj);
}

uint64_t xz_d_next(st_xz_d obj) {
	if (obj->rem) return 0;
	obj->rem = 12;
	uint64_t ts; uint32_t len;
	xz_d_read(obj, &ts, 8);//FIXME endian
	xz_d_read(obj, &len, 4);//FIXME endian
	obj->rem = len;
	return ts;
}

int xz_d_read(st_xz_d obj, void *out, unsigned len) {
	obj->ls.next_out = out;
	if (len > obj->rem) len = obj->rem;
	obj->ls.avail_out = len;
	while (1) {
		if (!obj->ls.avail_in) {
			obj->ls.next_in = obj->inb;
			obj->ls.avail_in = fread(obj->inb, 1, obj->inbl, obj->in);
		}
		if (LZMA_OK != lzma_code(&obj->ls, LZMA_RUN)) break;
		if (!obj->ls.avail_out) break;
	}
	obj->rem -= len - obj->ls.avail_out;
	return len - obj->ls.avail_out;
}
