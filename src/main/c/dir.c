#include <stdlib.h>
#include <windows.h>
#include "main.h"

//TODO UNIX
static void getRec(char *b1, char *b2,
		st_fentry *el, size_t *es, size_t *ep) {
	size_t len = strlen(b1);
	char *b1s = malloc(len + 3);
	memcpy(b1s, b1, len);
	if (b1s[len - 1] != '\\') b1s[len++] = '\\';
	b1s[len++] = '*';
	b1s[len] = 0;
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileExA(b1s, FindExInfoBasic, &fd,
			FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (h == INVALID_HANDLE_VALUE) { free(b1s); return; }
	b1s[--len] = 0;
	do {
		len = strlen(b2);
		char *eb2 = malloc(len + strlen(fd.cFileName) + 2);
		memcpy(eb2, b2, len);
		if (len) eb2[len++] = '/';
		strcpy(eb2 + len, fd.cFileName);
		char *eb1 = malloc(strlen(b1s) + strlen(fd.cFileName) + 1);
		strcpy(eb1, b1s);
		strcat(eb1, fd.cFileName);
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
				getRec(eb1, eb2, el, es, ep);
			free(eb1);
			free(eb2);
		} else {
			if (*ep >= *es) *el =
					realloc(*el, sizeof(sst_fentry) * (*es <<= 1));
			st_fentry cur = *el + (*ep)++;
			cur->name_real = eb1;
			cur->name_virt = eb2;
		}
	} while(FindNextFileA(h, &fd));
	FindClose(h);
	free(b1s);
}
static int cmpstfe(const void *p1, const void *p2) {
	return strcmp(((st_fentry)p1)->name_virt, ((st_fentry)p2)->name_virt);
}
st_raw raw_init(char *in) {
	st_raw ret = malloc(sizeof(sst_raw));
	memset(ret, 0, sizeof(sst_raw));
	ret->ecount = 16;
	ret->entries = malloc(sizeof(sst_fentry) * ret->ecount);
	getRec(in, "", &ret->entries, &ret->ecount, &ret->epos);
	ret->ecount = ret->epos;
	ret->epos = 0;
	ret->entries = realloc(ret->entries, sizeof(sst_fentry) * ret->ecount);
	qsort(ret->entries, ret->ecount, sizeof(sst_fentry), cmpstfe);
	return ret;
}
void raw_final(st_raw obj) {
	if (obj->mcr_tmp) free(obj->mcr_tmp);
	if (obj->out) free(obj->out);
	st_fentry e = obj->entries + obj->ecount;
	while (--e >= obj->entries) {
		free(e->name_real);
		free(e->name_virt);
	}
	free(obj->entries);
	free(obj);
}
static void *ext_zlib(void *src, size_t srcl, size_t *dstl) {
	*dstl = 1024 * 1024 * 16; //16MB
	void *dst = malloc(*dstl);
	z_stream zs = {};
	zs.next_in = src;
	zs.avail_in = srcl;
	zs.next_out = dst;
	zs.avail_out = *dstl;
	inflateInit(&zs);
	int res;
	while (1) {
		res = inflate(&zs, Z_NO_FLUSH);
		if (res == Z_STREAM_END) break;
		if (res != Z_OK || zs.avail_in == 0) break;
		if (zs.avail_out == 0) {
			dst = realloc(dst, *dstl << 1);
			zs.next_out = dst + *dstl;
			zs.avail_out = *dstl;
			*dstl <<= 1;
		}
	}
	inflateEnd(&zs);
	*dstl -= zs.avail_out;
	return realloc(dst, *dstl);
}
static void read_raw(st_raw obj, char *name) {
	size_t len = 1024 * 1024 * 16; //16MB
	obj->out = malloc(len);
	obj->len = 0;
	FILE *f = fopen(name, "rb");
	size_t tmp;
	while ((tmp = fread(obj->out + obj->len, 1, len - obj->len, f))) {
		obj->len += tmp;
		if (len <= obj->len)
			obj->out = realloc(obj->out, len <<= 2);
	}
	fclose(f);
	obj->out = realloc(obj->out, obj->len);
}
static void read_gzip(st_raw obj, char *name) {
	size_t len = 1024 * 1024 * 16; //16MB
	obj->out = malloc(len);
	obj->len = 0;
	gzFile f = gzopen(name, "rb");
	gzbuffer(f, 1024 * 1024); //1MB
	size_t tmp;
	while ((tmp = gzread(f, obj->out + obj->len, len - obj->len))) {
		obj->len += tmp;
		if (len <= obj->len)
			obj->out = realloc(obj->out, len <<= 2);
	}
	gzclose(f);
	obj->out = realloc(obj->out, obj->len);
}
static int is_gzip(char *name) {
	uint16_t tmp;
	FILE *f = fopen(name, "rb");
	fread(&tmp, 1, 2, f);
	fclose(f);
	return tmp == 0x8B1F;//FIXME endian
}
#define U8P(p, o) (*((uint8_t*)(((void*)(p)) + (o))))
#define rU32P(p, o) (*((uint32_t*)(((void*)(p)) + (o))))
//FIXME endian
#define U8_24(p, o) ((U8P(p, o) << 16) | (U8P(p, o + 1) << 8) \
		| (U8P(p, o + 2)))
#define U8_32(p, o) ((U8P(p, o) << 24) | (U8P(p, o + 1) << 16) \
		| (U8P(p, o + 2) << 8) | (U8P(p, o + 3)))
static int get_mcr(st_raw obj) {
	uint16_t i;
	size_t p;
	while (1) {
		i = DT2CP(obj->type) * 4;
		p = U8_24(obj->mcr_tmp, i) * 4096;
		obj->ts = rU32P(obj->mcr_tmp, i + 4096);
		if (p >= 8192) break;
		if (DT2CP(obj->type) >= 1023) return 0;
		obj->type += CP2DT(1);
	}
	dbgprintf("get_mcr %04X %I64d\n", i, p);
	obj->type &= ~DT_COMP;
	obj->type |= U8P(obj->mcr_tmp, p + 4);
	if (obj->out) free(obj->out);
	if (DT_IS(obj->type, DT_ZLIB))
		obj->out = ext_zlib(obj->mcr_tmp + p + 5,
				U8_32(obj->mcr_tmp, p), &obj->len);
	else
		obj->out = NULL; //FIXME extract gzip
	dbgprintf("GET_MCR\n");
	return 1;
}
int raw_do(st_raw obj) {
	dbgprintf("raw_do %04X %I64d %I64d %s\n", obj->type,
			obj->epos, obj->ecount, obj->name);
	if (DT_IS(obj->type, DT_MCR) && DT2CP(obj->type) < 1023) {
		dbgprintf("raw_do mcr continue\n");
		obj->type += CP2DT(1);
		return get_mcr(obj) ? 1 : raw_do(obj);
	}
	if (obj->epos >= obj->ecount) return 0;
	if (obj->mcr_tmp) { free(obj->mcr_tmp); obj->mcr_tmp = NULL; }
	if (obj->out) { free(obj->out); obj->out = NULL; }
	dbgprintf("raw_do new entry\n");
	obj->name = obj->entries[obj->epos].name_virt;
	if (is_gzip(obj->entries[obj->epos].name_real)) {
		obj->type = DT_GZIP;
		read_gzip(obj, obj->entries[obj->epos].name_real);
	} else {
		obj->type = 0;
		read_raw(obj, obj->entries[obj->epos].name_real);
	}
	obj->epos++;
	dbgprintf("raw_do new OK\n");
	if ((strstr(obj->name, ".mca") || strstr(obj->name, ".mcr"))
		&& obj->len >= 8192) {
		obj->type |= DT_MCR;
		obj->mcr_tmp = obj->out;
		obj->out = NULL;
		obj->mcr_len = obj->len;
		return get_mcr(obj) ? 1 : raw_do(obj);
	}
	return 1;
}
