#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "main.h"
#include "bsdiff/bsdiff.h"
typedef struct {
	char *fn;
	void *cd[1024];
	uint32_t ln[1024];
	uint8_t ty[1024];
	uint32_t ts[1024];
} mcr;
static void save_mcr(mcr *in) {
	uint8_t *of = malloc(4096);
	uint32_t total = 2;
	void *tmp; size_t tlen;
	int i;
	for (i = 0; i < 1024; i++) {
		if (!in->cd[i]) { *((uint32_t*)(of + i * 4)) = 0; continue; }
		of[i * 4] = total >> 16;
		of[i * 4 + 1] = total >> 8;
		of[i * 4 + 2] = total;
		//FIXME gzip
		tmp = zlib_def(in->cd[i], in->ln[i], &tlen);
		free(in->cd[i]);
		in->cd[i] = tmp; in->ln[i] = tlen;
		of[i * 4 + 3] = ceil((in->ln[i] + 5) / 4096.0);
		total += of[i * 4 + 3];
	}
	FILE *out = fopen(in->fn, "wb");
	fwrite(of, 1, 4096, out);
	fwrite(in->ts, 1, 4096, out);
	for (i = 0; i < 1024; i++) {
		if (!in->cd[i]) continue;
		in->ln[i]++;
		of[0] = in->ln[i] >> 24;
		of[1] = in->ln[i] >> 16;
		of[2] = in->ln[i] >> 8;
		of[3] = in->ln[i];
		fwrite(of, 1, 4, out);
		fwrite(in->ty + i, 1, 1, out);
		fwrite(in->cd[i], 1, in->ln[i] - 1, out);
		total = ceil((in->ln[i] += 4) / 4096.0);
		total *= 4096;
		total -= in->ln[i];
		memset(of, 0, 4096);
		while (total)
			total -= fwrite(of, 1, 4096 > total ? total : 4096, out);
		free(in->cd[i]);
	}
	fclose(out);
	free(of);
	free(in->fn);
	free(in);
}
static char *format_filename(const char *base, const char *name) {
	int i = strlen(base), j = strlen(name), k = i + j + 2;
	char *r = malloc(k);
	memset(r, 0, k);
	memcpy(r, base, i);
	MKDIR_PP(r);
	if (r[i - 1] != DIR_SEP) r[i++] = DIR_SEP;
	for (k = 0; k < j; k++)
		if (name[k] == '/') {
			MKDIR_PP(r);
			r[i++] = DIR_SEP;
		} else r[i++] = name[k];
	return r;
}
static mcr *load_mcr(const char *base, const char *inn) {
	mcr *out = malloc(sizeof(mcr));
	if (!out) return NULL;
	memset(out, 0, sizeof(mcr));
	out->fn = format_filename(base, inn);
	size_t ilen; void *in; uint8_t ct; uint32_t ts; void *tmp; size_t olen;
	file_read(out->fn, &in, &ilen);
	if (!in) return out;
	int i = 0;
	while (0 <= (i = get_mcr(in, ilen, i, &ct, &ts, &tmp, &olen))) {
		out->ty[i] = ct; out->ts[i] = ts;
		out->cd[i] = tmp; out->ln[i] = olen;
		if (++i > 1023) break;
	}
	free(in);
	return out;
}
static void build_main(void *srcn, int xz, char *dstn) {
	st_decomp src = dec_init(srcn, xz);
	mcr *pmcr = NULL;
	size_t rlen; ssize_t tlen; void *raw, *tmp; char *fn; int same;
	char *prev = NULL;
	while (dec_do(src)) {
		same = prev && !strcmp(src->name, prev);
		if (!same) {
			if (prev) free(prev);
			prev = malloc(strlen(src->name) + 1);
			strcpy(prev, src->name);
		}
		if (!same && pmcr) {
			save_mcr(pmcr); pmcr = NULL; }
		if (src->type & DT_MCR) {
			if (!same)
				pmcr = load_mcr(dstn, src->name);
			if (src->type & DT_BSDIFF) {
				raw  = pmcr->cd[DT2CP(src->type)];
				rlen = pmcr->ln[DT2CP(src->type)];
				tmp = bspatch(raw, rlen, src->out, src->len, &tlen);
			} else {
				tmp = src->out; tlen = src->len; src->out = NULL;
			}
			if (pmcr->cd[DT2CP(src->type)]) free(pmcr->cd[DT2CP(src->type)]);
			pmcr->cd[DT2CP(src->type)] = tmp;
			pmcr->ln[DT2CP(src->type)] = tlen;
			pmcr->ty[DT2CP(src->type)] = src->type & DT_COMP;
			pmcr->ts[DT2CP(src->type)] = src->ts;
		} else { //must !same
			fn = format_filename(dstn, src->name);
			if (src->type & DT_BSDIFF) {
				file_read(fn, &raw, &rlen);
				tmp = bspatch(raw, rlen, src->out, src->len, &tlen);
				free(raw);
			} else {
				tmp = src->out; tlen = src->len; src->out = NULL;
			}
			if (src->type & DT_GZIP) file_write_gz (fn, tmp, tlen);
			else                     file_write_raw(fn, tmp, tlen);
			free(tmp);
			free(fn);
		}
	}
	dec_final(src);
}

static int u64t_cmp(const void *a1, const void *a2) {
	return *((uint64_t*) a2) - *((uint64_t*) a1);
}

static uint64_t **sort_read(const char *indn) {
	uint64_t **ret = malloc(sizeof(uint64_t*) * 2);
	size_t rl0 = 0, rl1 = 0;
	ret[0] = malloc(sizeof(uint64_t));
	ret[1] = malloc(sizeof(uint64_t));
	size_t len = strlen(indn);
	char *inal = malloc(len + DIR_APP_LEN + 1), *inval;
	memcpy(inal, indn, len);
	gv_opendir(h, fd, inal, len, fdr);
	while (fdr) {
		if (gv_isdir(fdr)) goto next_read;
		const uint64_t tt = strtoull(gv_fname(fdr), &inval, 16);
		if (*inval) goto next_read;
		if (strlen(gv_fname(fdr)) == 16) {
			ret[0] = realloc(ret[0], (++rl0 + 1) * sizeof(uint64_t));
			ret[0][rl0 - 1] = tt;
		} else if (strlen(gv_fname(fdr)) == 12) {
			ret[1] = realloc(ret[1], (++rl1 + 1) * sizeof(uint64_t));
			ret[1][rl1 - 1] = tt;
		}
next_read:
		gv_readdir(h, fd, fdr);
	}
	gv_closedir(h);
	free(inal);
	ret[0][rl0] = 0;
	ret[1][rl1] = 0;
	qsort(ret[0], rl0, sizeof(uint64_t), u64t_cmp);
	qsort(ret[1], rl1, sizeof(uint64_t), u64t_cmp);
	return ret;
}
void build(char *in, char *out, uint64_t limit) {
	uint64_t **list = sort_read(in);
	size_t len = strlen(in);
	char *inv = malloc(len + 18);
	memcpy(inv, in, len);
	if (DIR_SEP != inv[len - 1]) inv[len++] = DIR_SEP;
	int i;
	for (i = 0; list[0][i] && list[0][i] > limit; i++) {
		sprintf(inv + len, "%016" PFI64 "X", list[0][i]);
		build_main(inv, 0, out);
	}
	uint64_t next;
	for (i = 0; list[1][i]; i++) {
		sprintf(inv + len, "%012" PFI64 "X", list[1][i]);
		st_xz_d xzs = xz_d_init(inv);
		while ((next = xz_d_next(xzs))) {
			if (next < limit) break;
			build_main(xzs, 1, out);
		}
		xz_d_final(xzs);
		if (next < limit) break;
	}
	free(inv);
	free(list[0]);
	free(list[1]);
	free(list);
}
