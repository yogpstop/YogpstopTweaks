#include <stdint.h>

typedef struct {
	uint64_t day;
	int sl;
	uint64_t *secs;
} days;

void xz_c_run(char*, size_t, days*, unsigned);
