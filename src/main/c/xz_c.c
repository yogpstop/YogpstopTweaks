#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lzma.h>
#include <zlib.h>
#include "main.h"

static int u64t_cmp(const void *a1, const void *a2) {
	return *((uint64_t*) a2) - *((uint64_t*) a1);
}

void xz_c_run(char *base, size_t blen, days *ds, unsigned dsl) {
	int i, j; FILE *inf1, *outfile; gzFile infile; lzma_stream ls;
	const size_t outbl = 1024 * 1024 * 16; void *outb = malloc(outbl);
	const size_t inbl = 1024 * 1024 * 16; void *inb = malloc(inbl);
	uint8_t hdr[12]; lzma_ret ret; lzma_options_lzma lz2opt = {};
	lzma_filter filters[] = {{LZMA_FILTER_LZMA2, &lz2opt},
			{LZMA_VLI_UNKNOWN,NULL}};
	lz2opt.dict_size = 1024 * 1024 * 128;
	lz2opt.lc = LZMA_LC_DEFAULT;
	lz2opt.lp = LZMA_LP_DEFAULT;
	lz2opt.pb = LZMA_PB_DEFAULT;
	lz2opt.mode = LZMA_MODE_NORMAL;
	lz2opt.nice_len = 273;
	lz2opt.mf = LZMA_MF_BT4;
	lz2opt.depth = 512;
	for (i = 0; i < dsl && !need_exit; i++) {
		qsort(ds[i].secs, ds[i].sl, sizeof(uint64_t), u64t_cmp);
		memset(&ls, 0, sizeof(lzma_stream));
		if (LZMA_OK != lzma_stream_encoder(&ls, filters, LZMA_CHECK_NONE))
			continue;
		ls.next_out = outb;
		ls.avail_out = outbl;
		sprintf(base + blen, "%012" PFI64 "X", ds[i].day);
		outfile = fopen(base, "wb");
		for (j = 0; j < ds[i].sl && !need_exit; j++) {
			sprintf(base + blen, "%016" PFI64 "X", ds[i].secs[j]);
			memcpy(hdr, ds[i].secs + j, 8);
			inf1 = fopen(base, "rb"); fseek(inf1, -4, SEEK_END);
			if (fread(hdr + 8, 1, 4, inf1)); fclose(inf1);
			infile = gzopen(base, "rb");
			gzbuffer(infile, 1024 * 1024 * 16);
			ls.next_in = hdr;
			ls.avail_in = 12;
			while (!need_exit) {
				if (!ls.avail_in) {
					ls.next_in = inb;
					ls.avail_in = gzread(infile, inb, inbl);
					if (!ls.avail_in) break;
				}
				if (LZMA_OK != lzma_code(&ls, LZMA_RUN)) break;
				if (!ls.avail_out) {
					fwrite(outb, 1, outbl, outfile);
					ls.next_out = outb;
					ls.avail_out = outbl;
				}
			}
			gzclose(infile);
		}
		ret = LZMA_OK;
		while (LZMA_STREAM_END != ret && !need_exit) {
			ret = lzma_code(&ls, LZMA_FINISH);
			if (LZMA_STREAM_END != ret && LZMA_OK != ret) break;
			if (!ls.avail_out) {
				fwrite(outb, 1, outbl, outfile);
				ls.next_out = outb;
				ls.avail_out = outbl;
			}
		}
		fwrite(outb, 1, outbl - ls.avail_out, outfile);
		fclose(outfile);
		lzma_end(&ls);
		if (!need_exit) for (j = 0; j < ds[i].sl; j++) {
			sprintf(base + blen, "%016" PFI64 "X", ds[i].secs[j]);
			remove(base);
		} else {
			sprintf(base + blen, "%012" PFI64 "X", ds[i].day);
			remove(base);
		}
	}
	free(inb);
	free(outb);
}
