#include "aamis.h"
#include <string.h>

AAMIS *aamis_open(void *p, ssize_t l) {
	AAMIS *ret = malloc(sizeof(AAMIS));
	if (!ret) return NULL;
	ret->pos = 0;
	ret->buf = p;
	ret->len = l;
	return ret;
}
void aamis_close(AAMIS *obj) {
	if (obj) free(obj);
}
void aamis_seek(AAMIS *obj, ssize_t pos) {
	if (!obj) return;
	obj->pos = pos;
}
ssize_t aamis_read(AAMIS *obj, void *buf, ssize_t len) {
	if (!obj || !obj->buf || !obj->len || !buf || !len) return 0;
	if (len > obj->len - obj->pos) len = obj->len - obj->pos;
	memcpy(buf, obj->buf + obj->pos, len);
	obj->pos += len;
	return len;
}
