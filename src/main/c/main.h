#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <zlib.h>

#ifdef DEBUG
#define dbgprintf(...) printf(__VA_ARGS__)
#else
#define dbgprintf(...)
#endif

#ifdef _WIN32
#include <windows.h>
#define DIR_APP_LEN 2
#define gv_opendir(v, e, n, l, r) \
		if (n[l - 1] != '\\') n[l++] = '\\'; n[l++] = '*'; n[l] = 0; \
		WIN32_FIND_DATAA e; \
		HANDLE v = FindFirstFileExA(n, FindExInfoBasic, &e, \
			FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH); \
		WIN32_FIND_DATAA *r = v != INVALID_HANDLE_VALUE ? &e : NULL; \
		n[--l] = 0;
#define gv_readdir(v, e, r) r = FindNextFile(v, &e) ? &e : NULL;
#define gv_closedir(v) FindClose(v);
#define gv_fname(r) r->cFileName
#define gv_isdir(r) (r->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#define PFI64 "I64"
#define PFZ "I"
#else
#include <dirent.h>
#define DIR_APP_LEN 0
#define gv_opendir(v, e, n, l, r) \
		if (n[l - 1] != '/') n[l++] = '/'; n[l] = 0; \
		struct dirent e, *r; DIR *v = opendir(n); gv_readdir(v, e, r);
#define gv_readdir(v, e, r) if (readdir_r(v, &e, &r)) r = NULL;
#define gv_closedir(v) closedir(v);
#define gv_fname(r) r->d_name
#define gv_isdir(r) (r->d_type & DT_DIR)
#if __LP64__
#define PFI64 "l"
#else
#define PFI64 "ll"
#endif
#define PFZ "z"
#endif

#define cast_struct(str, obj, mem) ((str*)(((void*)obj) - offsetof(str, mem)))
#define cast_comp(obj, mem) cast_struct(sst_compress, obj, mem)

typedef struct {
//PRIVATE
	gzFile f_out;
	char *prev;
} sst_compress, *st_compress;
st_compress comp_init(char*, int);
void comp_do(st_compress, uint16_t, char*, uint32_t, size_t, void*);
void comp_final(st_compress);

#define DT_IS(t, c) ((t) & (c))
#define DT_GZIP 0x01
#define DT_ZLIB 0x02
#define DT_COMP (DT_GZIP | DT_ZLIB)
#define DT_BSDIFF 0x08
#define DT_MCR 0x10
#define DT_NEW 0x20
#define DT_MAX 0x3F
// DT and CP type MUST BE uint16_t
#define DT2CP(type) ((type) >> 6)
#define CP2DT(cp) ((cp) << 6)

typedef struct {
//PRIVATE
	gzFile in_file;
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

typedef struct {
	uint64_t day;
	int sl;
	uint64_t *secs;
} days;
void xz_c_run(char*, size_t, days*, unsigned);
