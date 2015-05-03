#include <stdlib.h>

#define SEEK_SET -1
#define SEEK_CUR 0
#define SEEK_END 1

typedef struct {
	void *buf;
	ssize_t len;
	ssize_t al;
	ssize_t pos;
} AAMOS;
AAMOS *aamos_open(ssize_t);
void *aamos_close(AAMOS*, ssize_t*);
ssize_t aamos_tell(AAMOS*);
void aamos_seek(AAMOS*, ssize_t);
int aamos_write(AAMOS*, void*, ssize_t);
