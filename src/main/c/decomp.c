#include <stdlib.h>
#include <string.h>
#include "main.h"

st_decomp dec_init(char *in) {
	st_decomp ret = malloc(sizeof(sst_decomp));
	memset(ret, 0, sizeof(sst_decomp));
	ret->in_file = gzopen(in ,"rb");
	gzbuffer(ret->in_file, 1024 * 1024 * 16);//16MB buffer
	return ret;
}
// must be sizeof(size_t) * 8 < UINT8_MAX
static size_t read_len(st_decomp obj) {
	uint8_t len = 0, tmp;
	size_t ret = 0;
	while (!((ret >> (len * 7)) & 1)) {
		len += gzread(obj->in_file, &tmp, 1);// must return 1
		ret = (ret << 8) | tmp;
	}
	return ret & ~(1 << (len * 7));
}
int dec_do(st_decomp obj) {
	uint8_t tmp;
	if (!gzread(obj->in_file, &tmp, 1)) return 0;
	obj->type = tmp;
	if (DT_IS(obj->type, DT_NEW)) {
		size_t nlen = read_len(obj);
		if (obj->name) free(obj->name);
		obj->name = malloc(nlen + 1);
		gzread(obj->in_file, obj->name, nlen);
		obj->name[nlen] = 0;
		obj->type &= ~DT_NEW;
	}
	if (DT_IS(obj->type, DT_MCR)) {
		gzread(obj->in_file, &obj->ts, 4);// must return 4
		gzread(obj->in_file, &tmp, 1);// must return 1
		obj->type |= tmp << 8;
	}
	obj->len = read_len(obj);
	if (obj->out) free(obj->out);
	obj->out = malloc(obj->len);
	gzread(obj->in_file, obj->out, obj->len);
	return 1;
}
void dec_final(st_decomp obj) {
	gzclose(obj->in_file);
	if (obj->out) free(obj->out);
	if (obj->name) free(obj->name);
	free(obj);
}
