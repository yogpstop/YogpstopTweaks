#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "bsdiff/bsdiff.h"

#define GENCMP cmp = !prv ? 1 : !cur ? -1 : strcmp(prv->name, cur->name); \
		if (!cmp && prv->type & DT_MCR && cur->type & DT_MCR) \
		cmp = DT2CP(prv->type) - DT2CP(cur->type)

void loop(char *dir, char *sz, char *coc, char *cop, char **filter) {
	st_compress oc = comp_init(coc, 1), op = comp_init(cop, 0);
	st_decomp prv = NULL;
	if (sz) { prv = dec_init(sz, 0); dec_do(prv); }
	st_raw cur = raw_init(dir, filter); raw_do(cur);
	int GENCMP;
	while (prv || cur) {
		while (cmp < 0 && prv) {
			comp_do(op, prv->type, prv->name, prv->ts, prv->len, prv->out);
			if (!dec_do(prv)) { dec_final(prv); prv = NULL; cmp = 1; break; }
			GENCMP;
		}
		while (cmp > 0 && cur) {
			comp_do(oc, cur->type, cur->name, cur->ts, cur->len, cur->out);
			if (!raw_do(cur)) { raw_final(cur); cur = NULL; cmp = -1; break; }
			GENCMP;
		}
		if (cmp || !prv || !cur) continue;
		comp_do(oc, cur->type, cur->name, cur->ts, cur->len, cur->out);
		if (prv->type != cur->type ||
				(prv->type & DT_MCR && prv->ts != cur->ts) ||
				prv->len != cur->len || memcmp(prv->out, cur->out, prv->len)) {
			ssize_t diffl = 0;
			void *diff = NULL;
			if (prv->len > 4096 && cur->len > 4096) {
				diff = bsdiff(cur->out, cur->len,
						prv->out, prv->len, &diffl);
			}
			if (diff) {
				comp_do(op, prv->type | DT_BSDIFF,
						prv->name, prv->ts, diffl, diff); free(diff);
			} else
				comp_do(op, prv->type,
						prv->name, prv->ts, prv->len, prv->out);
		}
		if (!dec_do(prv)) { dec_final(prv); prv = NULL; cmp = 1; }
		if (!raw_do(cur)) { raw_final(cur); cur = NULL; cmp = -1; }
		GENCMP;
	}
	if (prv) dec_final(prv);
	if (cur) raw_final(cur);
	comp_final(op);
	comp_final(oc);
}
