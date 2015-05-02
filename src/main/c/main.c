#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "main.h"

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
	char *old = malloc(len + 17);
	memcpy(old, inal, len);
	old[len] = 0;
	old[len + 16] = 0;
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
		if (strlen(fd.cFileName) == 16 && (!old[len] ||
				memcmp(old + len, fd.cFileName, 16) < 0)) {
			memcpy(old + len, fd.cFileName, 16);
		}
	} while(FindNextFileA(h, &fd));
	FindClose(h);
	char *tmp = malloc(len + 6);
	memcpy(tmp, inal, len);
	strcpy(tmp + len, "Ztemp");
	char *new = malloc(len + 17);
	memcpy(new, inal, len);
	sprintf(new + len, "%016I64X", (unsigned long long int) time(NULL));
	loop(argv[1], old[len] ? old : NULL, new, tmp);
	if (old[len]) { remove(old); rename(tmp, old); }
	else remove(tmp);
	free(old);
	free(new);
	free(tmp);
	free(inal);
	return 0;
}
