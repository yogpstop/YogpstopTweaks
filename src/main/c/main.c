#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "main.h"

static int cmpcharpp(const void *p1, const void *p2) {
	return strcmp(*(char*const*)p1, *(char*const*)p2);
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
	size_t alen = 128, aindex = 0;
	char **array = malloc(sizeof(char*) * alen);
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		if (alen <= aindex)
			array = realloc(array, sizeof(char*) * (alen <<= 1));
		array[aindex] = malloc(len + strlen(fd.cFileName) + 1);
		memcpy(array[aindex], inal, len);
		strcpy(array[aindex++] + len, fd.cFileName);
	} while(FindNextFileA(h, &fd));
	FindClose(h);
	if (aindex < alen) array = realloc(array, sizeof(char*) * aindex);
	if (aindex) qsort(array, aindex, sizeof(char*), cmpcharpp);
	char *prev = aindex < 1 ? NULL : array[aindex - 1];
	char *tmp = malloc(len + 6);
	memcpy(tmp, inal, len);
	strcpy(tmp + len, "Ztemp");
	char *new = malloc(len + 17);
	memcpy(new, inal, len);
	sprintf(new + len, "%016I64X", (unsigned long long int) time(NULL));
	loop(argv[1], prev, new, tmp);
	if (prev) { remove(prev); rename(tmp, prev); }
	else remove(tmp);
	while (aindex--)
		free(array[aindex]);
	free(array);
	free(tmp);
	free(new);
	free(inal);
	return 0;
}
