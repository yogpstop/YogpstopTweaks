#ifndef NSZIP
#include <stdlib.h>
#include "main.h"

static void *szAlloc(void *obj, size_t s) {
	return malloc(s);
}
static void szFree(void *obj, void *buf) {
	free(buf);
}
ISzAlloc szMem = {szAlloc, szFree};
#endif
