#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include "main.h"

#define mc_rcon_free(p) free(((void*)(p)) - 12)

static char *mc_rcon_exec(int sock, int32_t *iid, int mode, const char *str, int stat) {
	int32_t len = strlen(str) + 9;
	int32_t *buf = malloc(len + 4);
	buf[0] = len; buf[1] = ++(*iid); buf[2] = mode;
	memcpy(buf + 3, str, len - 8);
	if (len + 4 != send(sock, (char*)buf, len + 4, 0)) {free(buf);return NULL;}
	if (12 != recv(sock, (char*)buf, 12, MSG_WAITALL)) {free(buf);return NULL;}
	len = buf[0] + 4;
	buf = realloc(buf, len);//FIXME realloc
	len -= 12;
	if (len != recv(sock, (char*)(buf + 3), len, MSG_WAITALL)) {free(buf);return NULL;}
	if (buf[1] != *iid || buf[2] != stat) {free(buf);return NULL;}
	return (char*) (buf + 3);
}
int create_socket(char *root, int32_t *iid) {
	char *buf, *ptr, *end;
	FILE *f; int sock;
	size_t len = strlen(root);
	// build filename
	buf = malloc(len + 19); if (!buf) return -1;
	memcpy(buf, root, len);
	if (buf[len - 1] != DIR_SEP) buf[len++] = DIR_SEP;
	memcpy(buf + len, "server.properties", 18);
	// read file
	f = fopen(buf, "rb"); free(buf); if (!f) return -1;
	fseek(f, 0, SEEK_END); len = ftell(f); rewind(f);
	buf = malloc(len + 1); if (!buf) { fclose(f); return -1; }
	if (len != fread(buf, 1, len, f)) { free(buf); fclose(f); return -1; }
	fclose(f); buf[len] = 0;
	// create socket
	ptr = strstr(buf, "rcon.port=");
	if (!ptr) { free(buf); return -1; }
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(strtoul(ptr + 10, NULL, 10)),
		.sin_addr.s_addr = 0x0100007F
	};
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sock) { free(buf); return -1; }
	if (-1 == connect(sock, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)))
			{ CLOSESOCKET(sock); free(buf); return -1; }
	// login
	ptr = strstr(buf, "rcon.password=");
	if (!ptr) { CLOSESOCKET(sock); free(buf); return -1; }
	end = strchr(ptr += 14, '\n');
	if (!end) { CLOSESOCKET(sock); free(buf); return -1; }
	if (end[-1] == '\r') end--;
	*end = 0;
	end = mc_rcon_exec(sock, iid, 3, ptr, 2);
	free(buf);
	if (!end) { CLOSESOCKET(sock); return -1; }
	mc_rcon_free(end);
	return sock;
}
char *mc_rcon_com(int sock, int32_t *iid, const char *str) {
	return mc_rcon_exec(sock, iid, 2, str, 0);
}
