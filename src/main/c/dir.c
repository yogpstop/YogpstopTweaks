#include <stdlib.h>
#include <string.h>
#include "main.h"

static void getRec(char *b1, char *b2,
		st_fentry *el, size_t *es, size_t *ep, char **filter) {
	size_t len = strlen(b1), fidx;
	char *b1s = malloc(len + DIR_APP_LEN + 1), *eb1, *eb2;
	memcpy(b1s, b1, len);
	gv_opendir(h, fd, b1s, len, fdr);
	while (fdr) {
		len = strlen(b2);
		eb2 = malloc(len + strlen(gv_fname(fdr)) + 2);
		memcpy(eb2, b2, len);
		if (len) eb2[len++] = '/';
		strcpy(eb2 + len, gv_fname(fdr));
		eb1 = malloc(strlen(b1s) + strlen(gv_fname(fdr)) + 1);
		strcpy(eb1, b1s);
		strcat(eb1, gv_fname(fdr));
		if (gv_isdir(fdr)) {
			if (strcmp(gv_fname(fdr), ".") && strcmp(gv_fname(fdr), ".."))
				getRec(eb1, eb2, el, es, ep, filter);
			free(eb1);
			free(eb2);
		} else {
			for (fidx = 0; filter[fidx]; fidx++) {
				if (!strncmp(eb2, filter[fidx] + 1, strlen(filter[fidx] + 1))) {
					fidx = filter[fidx][0] != '+';
					break;
				}
			}
			if (!fidx) {
				if (*ep >= *es) *el =
						realloc(*el, sizeof(sst_fentry) * (*es <<= 1));
				st_fentry cur = *el + (*ep)++;
				cur->name_real = eb1;
				cur->name_virt = eb2;
			} else { free(eb1); free(eb2); }
		}
		gv_readdir(h, fd, fdr);
	}
	gv_closedir(h);
	free(b1s);
}
static int cmpstfe(const void *p1, const void *p2) {
	return strcmp(((st_fentry)p1)->name_virt, ((st_fentry)p2)->name_virt);
}
st_raw raw_init(char *in, char **filter) {
	st_raw ret = malloc(sizeof(sst_raw));
	memset(ret, 0, sizeof(sst_raw));
	ret->ecount = 16;
	ret->entries = malloc(sizeof(sst_fentry) * ret->ecount);
	getRec(in, "", &ret->entries, &ret->ecount, &ret->epos, filter);
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
int raw_do(st_raw obj) {
	dbgprintf("raw_do %04X %"PFZ"u %"PFZ"u %s\n", obj->type,
			obj->epos, obj->ecount, obj->name);
	if (obj->out) { free(obj->out); obj->out = NULL; }
	if (obj->type & DT_MCR && DT2CP(obj->type) < 1023) {
		dbgprintf("raw_do mcr continue\n");
		uint8_t t;
		int i = get_mcr(obj->mcr_tmp, obj->mcr_len, DT2CP(obj->type) + 1,
				&t, &obj->ts, &obj->out, &obj->len);
		if (i < 0) return raw_do(obj);
		obj->type = (obj->type & (~DT_COMP & DT_MAX)) | t | CP2DT(i);
		return 1;
	}
	if (obj->epos >= obj->ecount) return 0;
	if (obj->mcr_tmp) { free(obj->mcr_tmp); obj->mcr_tmp = NULL; }
	dbgprintf("raw_do new entry\n");
	obj->name = obj->entries[obj->epos].name_virt;
	if (file_read(obj->entries[obj->epos].name_real, &obj->out, &obj->len))
		obj->type = DT_GZIP;
	else
		obj->type = 0;
	obj->epos++;
	dbgprintf("raw_do new OK\n");
	if ((strstr(obj->name, ".mca") || strstr(obj->name, ".mcr"))
		&& obj->len >= 8192) {
		obj->type |= DT_MCR;
		obj->mcr_len = obj->len;
		obj->mcr_tmp = obj->out;
		obj->out = NULL;
		uint8_t t;
		int i = get_mcr(obj->mcr_tmp, obj->mcr_len, DT2CP(obj->type) + 1,
				&t, &obj->ts, &obj->out, &obj->len);
		if (i < 0) return raw_do(obj);
		obj->type = (obj->type & (~DT_COMP & DT_MAX)) | t | CP2DT(i);
		return 1;
	}
	return 1;
}
