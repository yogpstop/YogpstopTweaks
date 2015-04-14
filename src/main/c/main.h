#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#ifndef NSZIP
#include <LzmaEnc.h>
#include <LzmaDec.h>
#endif

#define cast_struct(str, obj, mem) ((str*)(((void*)obj) - offsetof(str, mem)))
#define cast_comp(obj, mem) cast_struct(sst_compress, obj, mem)

#ifndef NSZIP
extern ISzAlloc szMem;
#endif
typedef struct {
//PRIVATE
#ifndef NSZIP
	CLzmaEncHandle z_h;
	ISeqInStream z_in;
	ISeqOutStream z_out;
#endif
	void *in_buf;
	size_t in_remain;
	FILE *f_out;
	char *prev;
} sst_compress, *st_compress;
st_compress comp_init(char*);
void comp_do(st_compress, uint16_t, char*, uint32_t, size_t, void*);
void comp_final(st_compress);

#define DT_IS(t, c) ((t) & (c))
#define DT_GZIP 0x01
#define DT_ZLIB 0x02
#define DT_COMP (DT_GZIP | DT_ZLIB)
#define DT_MCR 0x10
#define DT_NEW 0x20
#define DT_MAX 0x3F
// DT and CP type MUST BE uint16_t
#define DT2CP(type) ((type) >> 6)
#define CP2DT(cp) ((cp) << 6)

typedef struct {
//PRIVATE
#ifndef NSZIP
	CLzmaDec z_h;
	ELzmaStatus z_stat;
	void *in_buf;
	size_t in_total;
	size_t in_pos;
#endif
	FILE *in_file;
//PUBLIC
	uint16_t type;
	char *name;
	uint32_t ts;
	size_t len;
	void *out;
} sst_decomp, *st_decomp;
st_decomp dec_init(char*);
int dec_do(st_decomp);
void dec_final(st_decomp);

typedef struct {
	char *name_real;
	char *name_virt;
} sst_fentry, *st_fentry;
typedef struct {
//PRIVATE
	st_fentry entries;
	size_t ecount;
	size_t epos;
	void *mcr_tmp;
	size_t mcr_len;
//PUBLIC
	uint16_t type;
	char *name;
	uint32_t ts;
	size_t len;
	void *out;
} sst_raw, *st_raw;
st_raw raw_init(char*);
int raw_do(st_raw);
void raw_final(st_raw);

void loop(char*, char*, char*, char*);
