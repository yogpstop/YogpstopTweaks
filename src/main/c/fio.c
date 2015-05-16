#define YT_FIO_GZIP 1
#include "fio.h"
#undef YT_FIO_GZIP
#include "fio.h"
#include <stdint.h>
int file_read(const char *inn, void **out, size_t *olen) {
	uint16_t isgz;
	FILE *inf = fopen(inn, "rb");
	if (!inf || 2 > fread(&isgz, 1, 2, inf)) isgz = 0;
	if (inf) fclose(inf);
	isgz = isgz == 0x8B1F;//FIXME endian
	if (isgz) *out = file_read_gz (inn, olen);
	else      *out = file_read_raw(inn, olen);
	return isgz;
}
