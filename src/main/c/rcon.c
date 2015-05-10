#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define mc_rcon_free(p) free(((void*)(p)) - 12)

int create_socket(char *host, char *prot) {
	struct addrinfo *ais, *aip;
	int ret;
	if ((ret = getaddrinfo(host, prot, NULL, &ais))) return -1;
	int sock = -1;
	for (aip = ais; aip; aip = aip->ai_next) {
		aip->ai_socktype = SOCK_STREAM;
		sock = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		if (-1 == sock) continue;
		if (-1 != connect(sock, aip->ai_addr, aip->ai_addrlen)) break;
		close(sock); sock = -1;
	}
	freeaddrinfo(ais);
	return sock;
}
static char *exec(int sock, int32_t *iid, int mode, const char *str, int stat) {
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
int mc_rcon_login(int sock, int32_t *iid, const char *str) {
	str = exec(sock, iid, 3, str, 2);
	mc_rcon_free(str);
	return str ? 1 : 0;
}
char *mc_rcon_com(int sock, int32_t *iid, const char *str) {
	return exec(sock, iid, 2, str, 0);
}
