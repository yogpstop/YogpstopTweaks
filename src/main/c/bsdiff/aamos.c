#include "aamos.h"
#include <string.h>

AAMOS *aamos_open(ssize_t i) {
	if (i < 1) i = 1024;
	AAMOS *ret = malloc(sizeof(AAMOS));
	if (!ret) return NULL;
	ret->pos = ret->al = 0;
	ret->buf = malloc(ret->len = i);
	if (!ret->buf) { free(ret); return NULL; }
	return ret;
}
void *aamos_close(AAMOS *obj, ssize_t *len) {
	if (!obj) return NULL;
	void *tmp = realloc(obj->buf, obj->al);
	if (!tmp) tmp = obj->buf;
	*len = obj->al;
	free(obj);
	return tmp;
}
ssize_t aamos_tell(AAMOS *obj) {
	if (!obj) return 0;
	return obj->pos;
}
void aamos_seek(AAMOS *obj, ssize_t pos) {
	if (!obj) return;
	obj->pos = pos;
}
int aamos_write(AAMOS *obj, void *buf, ssize_t len) {
	if (!len) return 1;
	if (!obj || !obj->buf || !obj->len || obj->pos > obj->al || !buf) return 0;
	while (obj->pos + len > obj->len) {
		void *tmp = realloc(obj->buf, obj->len <<= 1);
		if (tmp) { obj->buf = tmp; continue; }
		free(obj->buf);
		obj->buf = NULL;
		obj->len = obj->al = obj->pos = 0;
		return 0;
	}
	if (obj->al < obj->pos + len) obj->al = obj->pos + len;
	memcpy(obj->buf + obj->pos, buf, len);
	obj->pos += len;
	return 1;
}
