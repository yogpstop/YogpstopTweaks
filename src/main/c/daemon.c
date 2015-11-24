#ifndef _WIN32
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "service.h"
volatile int need_exit = 0;
void sighdl(int sig) {
	need_exit = 1;
	(void) sig; // XXX
}
int main(int argc, char **argv) {
	if (argc == 3 && !strcmp(argv[1], "start")) {//TODO argc == 2 (read conf)
		if (!daemon(0, 0)) {
			//TODO pid file
			struct sigaction sact = {
				.sa_handler = sighdl
			};
			sigaction(SIGTERM, &sact, NULL);
			time_t next = time(NULL);
			while (!need_exit) {
				backup(argv[2]);
				while (next < time(NULL)) next += 60 * 20;
				while (time(NULL) < next && !need_exit) sleep(1);
			}
		}
	} else if (argc == 2 && !strcmp(argv[1], "stop")) {
		//TODO pid kill
	} else if (argc == 5 && !strcmp(argv[1], "rollback")) {
		char *inval;
		uint64_t ts = strtoull(argv[4], &inval, 16);
		if (*inval) return 2;
		build(argv[2], argv[3], ts);
	} else if (argc == 4 && !strcmp(argv[1], "restore")) {
		build_main(argv[2], 0, argv[3]);
	} else if (argc >= 4 && !strcmp(argv[1], "backup")) {
		loop(argv[2], NULL, argv[3], NULL, argv + 4);
	}
	return 0;
}
#endif
