#undef YT_FIO_RFUNC
#undef YT_FIO_WFUNC
#undef YT_FIO_TYPE
#undef YT_FIO_OPEN
#undef YT_FIO_CLOSE

#include <stdlib.h>
#if YT_FIO_GZIP
#include <zlib.h>
#define YT_FIO_RFUNC file_read_gz
#define YT_FIO_WFUNC file_write_gz
#define YT_FIO_TYPE gzFile
#define YT_FIO_OPEN gzopen
#define YT_FIO_CLOSE gzclose
#else
#include <stdio.h>
#define YT_FIO_RFUNC file_read_raw
#define YT_FIO_WFUNC file_write_raw
#define YT_FIO_TYPE FILE*
#define YT_FIO_OPEN fopen
#define YT_FIO_CLOSE fclose
#endif

static void *YT_FIO_RFUNC(const char *inn, size_t *olen) {
	*olen = 0;
	if (!inn) return NULL;
	YT_FIO_TYPE inf = YT_FIO_OPEN(inn, "rb");
	if (!inf) return NULL;
#if YT_FIO_GZIP
	gzbuffer(inf, 1024 * 1024); //1MB
#endif
	size_t len = 1024 * 1024 * 16; //16MB
	void *out = malloc(len), *ort;
	if (!out) { YT_FIO_CLOSE(inf); return NULL; }
	size_t tmp;
#if YT_FIO_GZIP
	while ((tmp = gzread(inf, out + *olen, len - *olen))) {
#else
	while ((tmp = fread(out + *olen, 1, len - *olen, inf))) {
#endif
		*olen += tmp;
		if (len <= *olen) {
			ort = realloc(out, len <<= 1);
			if (!ort) { *olen = 0; free(out); YT_FIO_CLOSE(inf); return NULL; }
			out = ort;
		}
	}
	YT_FIO_CLOSE(inf);
	out = realloc(out, *olen);
	return out;
}

void YT_FIO_WFUNC(const char *outn, const void *in, const size_t ilen) {
	if (!outn || !in) return;
	YT_FIO_TYPE out = YT_FIO_OPEN(outn, "wb");
	if (!out) return;
#if YT_FIO_GZIP
	gzbuffer(out, 1024 * 1024); //1MB
#endif
	size_t tmp, ipos = 0;
#if YT_FIO_GZIP
	while ((tmp = gzwrite(out, in + ipos, ilen - ipos))) {
#else
	while ((tmp = fwrite(in + ipos, 1, ilen - ipos, out))) {
#endif
		ipos += tmp; }
	YT_FIO_CLOSE(out);
}
