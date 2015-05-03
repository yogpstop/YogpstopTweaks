#include <stdlib.h>

#define SEEK_SET -1
#define SEEK_CUR 0
#define SEEK_END 1

typedef struct {
	void *buf;
	ssize_t len;
	ssize_t pos;
} AAMIS;
AAMIS *aamis_open(void*, ssize_t);
void aamis_close(AAMIS*);
void aamis_seek(AAMIS*, ssize_t);
ssize_t aamis_read(AAMIS*, void*, ssize_t);
