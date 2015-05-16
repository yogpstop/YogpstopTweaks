#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
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
		if (!in->cd[i]) continue;
		of[i * 4] = total << 16;
		of[i * 4 + 1] = total << 8;
		of[i * 4 + 2] = total;
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
		of[0] = in->ln[i] << 24;
		of[1] = in->ln[i] << 16;
		of[2] = in->ln[i] << 8;
		of[3] = in->ln[i];
		fwrite(of, 1, 4, out);
		fwrite(in->ty + i, 1, 1, out);
		fwrite(in->cd[i], 1, in->ln[i] - 1, out);
		free(in->cd[i]);
	}
	fclose(out);
	free(of);
	free(in);
}
static mcr *load_mcr(const char *inn) {
	mcr *out = malloc(sizeof(mcr));
	if (!out) return NULL;
	memset(out, 0, sizeof(mcr));
	out->fn = malloc(strlen(inn) + 1);
	strcpy(out->fn, inn);
	size_t ilen; void *in; uint8_t ct; uint32_t ts; void *tmp; size_t olen;
	file_read(inn, &in, &ilen);
	if (!in) return out;
	int i;
	while (0 <= (i = get_mcr(in, ilen, i, &ct, &ts, &tmp, &olen))) {
		out->ty[i] = ct; out->ts[i] = ts; out->cd[i] = tmp; out->ln[i] = olen;
	}
	return out;
}
void build_main(char *srcn, char *dstn) {
	st_decomp src = dec_init(srcn);
	mcr *pmcr = NULL;
	size_t rlen; ssize_t tlen; void *raw, *tmp;
	while (dec_do(src)) {
		if (src->type & DT_NEW && pmcr) {
			save_mcr(pmcr); pmcr = NULL; }
		if (src->type & DT_MCR) {
			if (src->type & DT_NEW)
				pmcr = load_mcr(src->name);
			if (src->type & DT_BSDIFF) {
				raw = zlib_inf(pmcr->cd[DT2CP(src->type)],
						pmcr->ln[DT2CP(src->type)], &rlen);
				tmp = bspatch(raw, rlen, src->out, src->len, &tlen);
				free(raw);
			} else {
				tmp = src->out; tlen = src->len; src->out = NULL;
			}
			if (pmcr->cd[DT2CP(src->type)]) free(pmcr->cd[DT2CP(src->type)]);
			pmcr->cd[DT2CP(src->type)] = tmp;
			pmcr->ln[DT2CP(src->type)] = tlen;
			pmcr->ty[DT2CP(src->type)] = src->type & DT_COMP;
			pmcr->ts[DT2CP(src->type)] = src->ts;
		} else if (src->type & DT_NEW) {
			if (src->type & DT_BSDIFF) {
				file_read(src->name, &raw, &rlen);
				tmp = bspatch(raw, rlen, src->out, src->len, &tlen);
				free(raw);
			} else {
				tmp = src->out; tlen = src->len;
			}
			if (src->type & DT_GZIP) file_write_gz (src->name, tmp, tlen);
			else                     file_write_raw(src->name, tmp, tlen);
		// } else { impossible
		}
	}
	dec_final(src);
}
