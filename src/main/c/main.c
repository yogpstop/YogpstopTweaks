#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <windows.h>
#include "main.h"
#include "day.h"

static int dcmp(const void *m1, const void *m2) {
	return ((days*)m1)->day - ((days*)m2)->day;
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	size_t len = strlen(argv[2]);
	char *inal = malloc(len + 3);
	memcpy(inal, argv[2], len);
	if (inal[len - 1] != '\\') inal[len++] = '\\';
	inal[len++] = '*';
	inal[len] = 0;
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileExA(inal, FindExInfoBasic, &fd,
			FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (h == INVALID_HANDLE_VALUE) { free(inal); return 2; }
	inal[--len] = 0;
	uint64_t now = time(NULL);
	char *inval;
	char *new = malloc(len + 17);
	memcpy(new, inal, len);
	sprintf(new + len, "%016I64X", now);
	char *old = malloc(len + 17);
	memcpy(old, inal, len);
	old[len] = 0;
	old[len + 16] = 0;
	now /= 60 * 60 * 24;
	unsigned cupl = 0;
	days *cup = NULL;
	days key = {0, 0, NULL};
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		if (strlen(fd.cFileName) != 16) continue;
		const uint64_t tt = strtoull(fd.cFileName, &inval, 16);
		if (*inval) continue;
		if (!old[len] || memcmp(old + len, fd.cFileName, 16) < 0)
			memcpy(old + len, fd.cFileName, 16);
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
	} while(FindNextFileA(h, &fd));
	FindClose(h);
	char *tmp = malloc(len + 6);
	memcpy(tmp, inal, len);
	strcpy(tmp + len, "Ztemp");
	loop(argv[1], old[len] ? old : NULL, new, tmp);
	if (old[len]) { remove(old); rename(tmp, old); }
	else remove(tmp);
	xz_c_run(new, len, cup, cupl);
	free(tmp);
	free(old);
	free(new);
	free(inal);
	return 0;
}
