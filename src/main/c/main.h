#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <LzmaEnc.h>
#include <LzmaDec.h>

#define cast_struct(str, obj, mem) ((str*)(((void*)obj) - offsetof(str, mem)))
#define cast_comp(obj, mem) cast_struct(sst_compress, obj, mem)

extern ISzAlloc szMem;
typedef struct {
//PRIVATE
	CLzmaEncHandle z_h;
	ISeqInStream z_in;
	ISeqOutStream z_out;
	FILE *f_out;
	void *in_buf;
	size_t in_remain;
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
	CLzmaDec z_h;
	ELzmaStatus z_stat;
	FILE *in_file;
	void *in_buf;
	size_t in_total;
	size_t in_pos;
//PUBLIC
	uint16_t type;
	char *name;
	uint32_t ts;
	size_t len;
	void *out;
} sst_decomp, *st_decomp;
#define dec_done(st) ((st)->z_stat == LZMA_STATUS_FINISHED_WITH_MARK)
st_decomp dec_init(char*);
void dec_do(st_decomp);
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
#define raw_done(st) \
		((!DT_IS((st)->type, DT_MCR) || DT2CP((st)->type) >= 1023) \
		&& (st)->epos >= (st)->ecount)
st_raw raw_init(char*);
void raw_do(st_raw);
void raw_final(st_raw);

void loop(char*, char*, char*, char*);
