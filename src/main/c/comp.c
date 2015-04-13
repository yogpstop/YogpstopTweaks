#include <stdlib.h>
#include <string.h>
#include "main.h"

static SRes intern_read(void *obj, void *buf, size_t *siz) {
	st_compress ci = cast_comp(obj, z_in);
	size_t amo = ci->in_remain;
	if (amo > *siz) amo = *siz;
	memcpy(buf, ci->in_buf, amo);
	ci->in_buf += amo;
	ci->in_remain -= amo;
	*siz = amo;
	return SZ_OK;
}
static size_t intern_write(void *obj, const void *buf, size_t siz) {
	return fwrite(buf, 1, siz, cast_comp(obj, z_out)->f_out);
}
void comp_final(st_compress obj) {
	LzmaEnc_Destroy(obj->z_h, &szMem, &szMem);
	fclose(obj->f_out);
	if (obj->prev) free(obj->prev);
	free(obj);
}
st_compress comp_init(char *out) {
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = 9;
	props.dictSize = 1024 * 1024 * 128;
	// props.reduceSize
	// props.lc
	// props.lp
	// props.pb
	props.algo = 1;
	props.fb = 273;
	props.btMode = 1;
	props.numHashBytes = 4;
	props.mc = 1 << 30;
	props.writeEndMark = 0;
	props.numThreads = 2;
	LzmaEncProps_Normalize(&props);
	st_compress ret = malloc(sizeof(sst_compress));
	memset(ret, 0, sizeof(sst_compress));
	ret->z_h  = LzmaEnc_Create(&szMem);
	LzmaEnc_SetProps(ret->z_h, &props);
	ret->z_in.Read = intern_read;
	ret->z_out.Write = intern_write;
	ret->f_out = fopen(out, "wb");
	size_t psize = LZMA_PROPS_SIZE;
	void *prop = malloc(psize);
	LzmaEnc_WriteProperties(ret->z_h, prop, &psize);
	fwrite(prop, 1, psize, ret->f_out);
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
	int sname = obj->prev && !strcmp(name, obj->prev);
	obj->in_remain = 1 + (sname ? 0 : st_len(strlen(name)) + strlen(name))
			+ (DT_IS(type, DT_MCR) ? 5 : 0) + st_len(len) + len;
	obj->in_buf = malloc(obj->in_remain);
	void *cur = obj->in_buf;
	*((uint8_t*)cur++) = (uint8_t) (type | (sname ? 0 : DT_NEW));
	if (!sname) {
		if (obj->prev) free(obj->prev);
		size_t nlen = strlen(name);
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
	cur = obj->in_buf;
	LzmaEnc_Encode(obj->z_h, &obj->z_out, &obj->z_in, NULL, &szMem, &szMem);
	free(cur);
}
