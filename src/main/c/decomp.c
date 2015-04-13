#include <stdlib.h>
#include <string.h>
#include "main.h"

st_decomp dec_init(char *in) {
	st_decomp ret = malloc(sizeof(sst_decomp));
	memset(ret, 0, sizeof(sst_decomp));
	ret->in_file = fopen(in ,"rb");
	void *prop = malloc(LZMA_PROPS_SIZE);
	fread(prop, 1, LZMA_PROPS_SIZE, ret->in_file);
	LzmaDec_Construct(&ret->z_h);
	LzmaDec_Allocate(&ret->z_h, prop, LZMA_PROPS_SIZE, &szMem);
	LzmaDec_Init(&ret->z_h);
	free(prop);
	ret->in_total = 1024 * 1024 * 16; // 16MB buffer
	ret->in_buf = malloc(ret->in_total);
	return ret;
}
static size_t zread(void *ptr, size_t len, st_decomp obj) {
	void *dst = ptr;
	size_t dst_cur;
	size_t src_cur;
	while (ptr + len > dst && obj->z_stat != LZMA_STATUS_FINISHED_WITH_MARK) {
		dst_cur = len - (dst - ptr);
		src_cur = obj->in_pos;
		LzmaDec_DecodeToBuf(&obj->z_h, dst, &dst_cur,
				obj->in_buf, &src_cur, LZMA_FINISH_ANY, &obj->z_stat);
		memmove(obj->in_buf, obj->in_buf + src_cur, obj->in_pos -= src_cur);
		if (obj->in_total <= obj->in_pos)
			obj->in_buf = realloc(obj->in_buf, (obj->in_total <<= 1));
		obj->in_pos += fread(obj->in_buf + obj->in_pos, 1,
			obj->in_total - obj->in_pos, obj->in_file);
		dst += dst_cur;
	}
	return dst - ptr;
}
// must be sizeof(size_t) * 8 < UINT8_MAX
static size_t read_len(st_decomp obj) {
	uint8_t len = 0;
	size_t ret = 0;
	uint8_t tmp;
	while (!((ret >> (len * 7)) & 1)) {
		len += zread(&tmp, 1, obj);// must return 1
		ret = (ret << 8) | tmp;
	}
	ret &= ~(1 << (len * 7));
	return ret;
}
void dec_do(st_decomp obj) {
	uint8_t tmp;
	zread(&tmp, 1, obj);// must return 1
	obj->type = tmp;
	if (DT_IS(obj->type, DT_NEW)) {
		size_t nlen = read_len(obj);
		if (obj->name) free(obj->name);
		obj->name = malloc(nlen + 1);
		zread(obj->name, nlen, obj);
		obj->name[nlen] = 0;
		obj->type &= ~DT_NEW;
	}
	if (DT_IS(obj->type, DT_MCR)) {
		zread(&obj->ts, 4, obj);// must return 4
		zread(&tmp, 1, obj);// must return 1
		obj->type |= tmp << 8;
	}
	obj->len = read_len(obj);
	if (obj->out) free(obj->out);
	obj->out = malloc(obj->len);
	zread(obj->out, obj->len, obj);
}
void dec_final(st_decomp obj) {
	LzmaDec_Free(&obj->z_h, &szMem);
	fclose(obj->in_file);
	free(obj->in_buf);
	if (obj->out) free(obj->out);
	if (obj->name) free(obj->name);
	free(obj);
}
