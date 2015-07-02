#ifdef _WIN32
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include "service.h"
#define SVCNAME "McRconBackup"
#define DISPNAME "Minecraft RCON Backup Service"
volatile int need_exit = 0;
static SERVICE_STATUS g_srv_stat = {
	SERVICE_WIN32_OWN_PROCESS, SERVICE_START_PENDING,
	SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN, NO_ERROR, NO_ERROR, 0, 0};
static SERVICE_STATUS_HANDLE g_srv_stat_h;
static DWORD main_thread_id;
static void WINAPI SvcHandler(DWORD cc) {
	switch (cc) {
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		need_exit = 1;
		g_srv_stat.dwCurrentState = SERVICE_STOP_PENDING;
		g_srv_stat.dwCheckPoint = 0;
		g_srv_stat.dwWaitHint = 3000;
		HANDLE thread = OpenThread(SYNCHRONIZE, FALSE, main_thread_id);
		if (!thread) break;
		do {
			g_srv_stat.dwCheckPoint++;
			SetServiceStatus(g_srv_stat_h, &g_srv_stat);
		} while (WaitForSingleObject(thread, 2000) != WAIT_OBJECT_0);
		CloseHandle(thread);
		g_srv_stat.dwCurrentState = SERVICE_STOPPED;
		g_srv_stat.dwCheckPoint = 0;
		g_srv_stat.dwWaitHint = 0;
		break;
	}
	SetServiceStatus(g_srv_stat_h, &g_srv_stat);
}
static char *cfile;
static void WINAPI SvcMain(DWORD argc, char **argv) {
	main_thread_id = GetCurrentThreadId();
	g_srv_stat_h = RegisterServiceCtrlHandlerA(SVCNAME, SvcHandler);
	g_srv_stat.dwCurrentState = SERVICE_RUNNING;
	g_srv_stat.dwCheckPoint = 0;
	g_srv_stat.dwWaitHint = 0;
	SetServiceStatus(g_srv_stat_h, &g_srv_stat);
	WSADATA wsad; WSAStartup(WINSOCK_VERSION, &wsad);
	time_t next = time(NULL);
	while (!need_exit) {
		backup(cfile);
		while (next < time(NULL)) next += 60 * 20;
		while (time(NULL) < next && !need_exit) sleep(1);
	}
	WSACleanup();
}
static void SvcInstall(char *fn) {
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	char argv0[MAX_PATH + strlen(fn) + 9];
	argv0[0] = '\"';
	DWORD pos = GetModuleFileNameA(NULL, argv0 + 1, MAX_PATH) + 1;
	strcpy(argv0 + pos, "\" daemon \"");
	strcat(argv0, fn);
	strcat(argv0, "\"");
	SC_HANDLE svc = CreateServiceA(scm, SVCNAME, DISPNAME, 0,
		SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		argv0, NULL, NULL, NULL, NULL, NULL);
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
}
static void SvcUninstall() {
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	SC_HANDLE svc = OpenServiceA(scm, SVCNAME, DELETE);
	DeleteService(svc);
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
}
static void SvcStart() {
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	SC_HANDLE svc = OpenServiceA(scm, SVCNAME, SERVICE_START);
	StartService(svc, 0, NULL);
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
}
static void SvcStop() {
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	SC_HANDLE svc = OpenServiceA(scm, SVCNAME, SERVICE_STOP);
	ControlService(svc, SERVICE_CONTROL_STOP, &g_srv_stat);
	CloseServiceHandle(svc);
	CloseServiceHandle(scm);
}
static const SERVICE_TABLE_ENTRYA s_table[] = {
	{SVCNAME, SvcMain}, {NULL, NULL} };
int main(int argc, char **argv) {
	if (argc == 3 && !strcmp(argv[1], "daemon")) {
		cfile = argv[2];
		StartServiceCtrlDispatcherA(s_table);
	} else if (argc == 3 && !strcmp(argv[1], "install")) {
		SvcInstall(argv[2]);
	} else if (argc == 2 && !strcmp(argv[1], "uninstall")) {
		SvcUninstall();
	} else if (argc == 2 && !strcmp(argv[1], "start")) {
		SvcStart();
	} else if (argc == 2 && !strcmp(argv[1], "stop")) {
		SvcStop();
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
