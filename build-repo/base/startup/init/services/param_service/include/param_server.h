#ifndef LE_SOCKET_H
#define LE_SOCKET_H
#include <stdint.h>
#include "param_utils.h"
#include "parameter.h"

#define LOOP_MAX_CLIENT 1024
#define LOOP_MAX_SOCKET 1024

struct EventArgs {
	int epollFd;
	int clientFd;
};

enum {
	SOCK_UNKNOWN = -1,
	SOCK_DISCONNECTED,
	SOCK_CONNECTED,
};

void ParamServerStart();
#endif // LE_SOCKET_H
