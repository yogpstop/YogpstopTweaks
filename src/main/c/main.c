#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include "main.h"

static int dcmp(const void *m1, const void *m2) {
	return ((days*)m1)->day - ((days*)m2)->day;
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	size_t len = strlen(argv[2]);
	char *inal = malloc(len + DIR_APP_LEN + 1);
	memcpy(inal, argv[2], len);
	gv_opendir(h, fd, inal, len, fdr);
	uint64_t now = time(NULL);
	char *inval;
	char *new = malloc(len + 17);
	memcpy(new, inal, len);
	sprintf(new + len, "%016" PFI64 "X", now);
	char *old = malloc(len + 17);
	memcpy(old, inal, len);
	old[len] = 0;
	old[len + 16] = 0;
	now /= 60 * 60 * 24;
#ifdef _WIN32
	unsigned cupl = 0;
#else
	size_t cupl = 0;
#endif
	days *cup = NULL;
	days key = {0, 0, NULL};
	while (fdr) {
		if (gv_isdir(fdr)) continue;
		if (strlen(gv_fname(fdr)) != 16) continue;
		const uint64_t tt = strtoull(gv_fname(fdr), &inval, 16);
		if (*inval) continue;
		if (!old[len] || memcmp(old + len, gv_fname(fdr), 16) < 0)
			memcpy(old + len, gv_fname(fdr), 16);
		key.day = tt / (60 * 60 * 24);
		if (key.day >= now) continue;
		days *de = lfind(&key, cup, &cupl, sizeof(days), dcmp);
		if (!de) {
			cup = realloc(cup, ++cupl * sizeof(days));
			memcpy(cup + cupl - 1, &key, sizeof(days));
			de = cup + cupl - 1;
		}
		de->secs = realloc(de->secs, ++de->sl * sizeof(uint64_t));
		de->secs[de->sl - 1] = tt;
		gv_readdir(h, fd, fdr);
	}
	gv_closedir(h);
	char *tmp = malloc(len + 6);
	memcpy(tmp, inal, len);
	strcpy(tmp + len, "Ztemp");
	loop(argv[1], old[len] ? old : NULL, new, tmp);
	if (old[len]) { remove(old); rename(tmp, old); }
	else remove(tmp);
	xz_c_run(new, len, cup, cupl);
	while (cupl--) free(cup[cupl].secs);
	free(cup);
	free(tmp);
	free(old);
	free(new);
	free(inal);
	return 0;
}
