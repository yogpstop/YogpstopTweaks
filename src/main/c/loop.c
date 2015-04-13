#include <stdlib.h>
#include <string.h>
#include "main.h"

#define GENCMP cmp = !prv ? 1 : !cur ? -1 : strcmp(prv->name, cur->name); \
		if (!cmp && DT_IS(prv->type, DT_MCR) && DT_IS(cur->type, DT_MCR)) \
		cmp = DT2CP(prv->type) - DT2CP(cur->type)

void loop(char *dir, char *sz, char *coc, char *cop) {
	st_compress oc = comp_init(coc), op = comp_init(cop);
	st_decomp prv = NULL;
	if (sz) { prv = dec_init(sz); dec_do(prv); }
	st_raw cur = raw_init(dir); raw_do(cur);
	int GENCMP;
	while (prv || cur) {
		while (cmp < 0 && prv) {
			comp_do(op, prv->type, prv->name, prv->ts, prv->len, prv->out);
			if (dec_done(prv)) { dec_final(prv); prv = NULL; cmp = 1; break; }
			dec_do(prv);
			GENCMP;
		}
		while (cmp > 0 && cur) {
			comp_do(oc, cur->type, cur->name, cur->ts, cur->len, cur->out);
			if (raw_done(cur)) { raw_final(cur); cur = NULL; cmp = -1; break; }
			raw_do(cur);
			GENCMP;
		}
		if (cmp || !prv || !cur) continue;
		comp_do(oc, cur->type, cur->name, cur->ts, cur->len, cur->out);
		if (prv->type != cur->type ||
				(DT_IS(prv->type, DT_MCR) && prv->ts != cur->ts) ||
				prv->len != cur->len || memcmp(prv->out, cur->out, prv->len)) {
			comp_do(op, prv->type, prv->name, prv->ts, prv->len, prv->out);
		}
		if (dec_done(prv)) { dec_final(prv); prv = NULL; cmp = 1; }
		else dec_do(prv);
		if (raw_done(cur)) { raw_final(cur); cur = NULL; cmp = -1; }
		else raw_do(cur);
		GENCMP;
	}
	if (prv) dec_final(prv);
	if (cur) raw_final(cur);
	comp_final(op);
	comp_final(oc);
}
