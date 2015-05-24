#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <unistd.h>
#if _WIN32
#include <winsock2.h>
#endif
#include "main.h"

static int dcmp(const void *m1, const void *m2) {
	return ((days*)m1)->day - ((days*)m2)->day;
}

static void run_wrap(char *target_dir, char *dest_dir, char **filter) {
	size_t len = strlen(dest_dir);
	char *inal = malloc(len + DIR_APP_LEN + 1);
	memcpy(inal, dest_dir, len);
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
		if (gv_isdir(fdr)) goto next_read;
		if (strlen(gv_fname(fdr)) != 16) goto next_read;
		const uint64_t tt = strtoull(gv_fname(fdr), &inval, 16);
		if (*inval) goto next_read;
		if (!old[len] || memcmp(old + len, gv_fname(fdr), 16) < 0)
			memcpy(old + len, gv_fname(fdr), 16);
		key.day = tt / (60 * 60 * 24);
		if (key.day >= now) goto next_read;
		days *de = lfind(&key, cup, &cupl, sizeof(days), dcmp);
		if (!de) {
			cup = realloc(cup, ++cupl * sizeof(days));
			memcpy(cup + cupl - 1, &key, sizeof(days));
			de = cup + cupl - 1;
		}
		de->secs = realloc(de->secs, ++de->sl * sizeof(uint64_t));
		de->secs[de->sl - 1] = tt;
next_read:
		gv_readdir(h, fd, fdr);
	}
	gv_closedir(h);
	char *tmp = malloc(len + 6);
	memcpy(tmp, inal, len);
	strcpy(tmp + len, "Ztemp");
	loop(target_dir, old[len] ? old : NULL, new, tmp, filter);
	if (old[len]) { remove(old); rename(tmp, old); }
	else remove(tmp);
	xz_c_run(new, len, cup, cupl);
	while (cupl--) free(cup[cupl].secs);
	free(cup);
	free(tmp);
	free(old);
	free(new);
	free(inal);
}
static void rcon_wrap(char *ip, char *port, char *pw,
		char *target, char *dest, char **filter) {
	int sock = create_socket(ip, port);
	int32_t iid = 0;
	if (!mc_rcon_login(sock, &iid, pw)) { close(sock); return; }
	char *r = mc_rcon_com(sock, &iid, "save-off"); if (r) mc_rcon_free(r);
	r = mc_rcon_com(sock, &iid, "save-all flush"); if (r) mc_rcon_free(r);
	run_wrap(target, dest, filter);
	r = mc_rcon_com(sock, &iid, "save-on"); if(r) mc_rcon_free(r);
	close(sock);
}
static void config_wrap(char *cfile) {
	FILE *f = fopen(cfile, "rb"); if (!f) return;
	fseek(f, 0, SEEK_END); long l = ftell(f); rewind(f);
	char *buf = malloc(l + 1); if(!buf) { fclose(f); return; }
	if (l != fread(buf, 1, l, f)) { free(buf); fclose(f); return; }
	fclose(f);
	buf[l] = 0;
	char *tp1, *tp2, *rp1, *rp2;
	char **arg = NULL;
	rp1 = strtok_r(buf, "\r\n", &tp1);
	while(rp1) {
		l = 0;
		rp2 = strtok_r(rp1, ";", &tp2);
		while (rp2) {
			arg = realloc(arg, (l + 1) * sizeof(char*));
			arg[l++] = rp2;
			rp2 = strtok_r(NULL, ";", &tp2);
		}
		if (l > 5) {
			arg = realloc(arg, (l + 1) * sizeof(char*));
			arg[l] = NULL;
			rcon_wrap(arg[0], arg[1], arg[2], arg[3], arg[4], arg + 5);
		}
		rp1 = strtok_r(NULL, "\r\n", &tp1);
	}
	free(arg);
	free(buf);
}
int main(int argc, char **argv) {
	if (argc == 4) {
		char *inval;
		uint64_t ts = strtoull(argv[3], &inval, 16);
		if (*inval) return 2;
		build(argv[1], argv[2], ts);
		return 0;
	}
	if (argc != 2) return 1;
#if _WIN32
	WSADATA wsad; WSAStartup(WINSOCK_VERSION, &wsad);
#endif
	while (1) {
		config_wrap(argv[1]);
		sleep(60 * 20);
	}
#if _WIN32
	WSACleanup();
#endif
	return 0;
}
