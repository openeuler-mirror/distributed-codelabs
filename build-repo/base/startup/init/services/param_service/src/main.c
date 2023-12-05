#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <string.h>
#include "beget_ext.h"

int main(int argc, char* argv[])
{
	signal(SIGPIPE, SIG_IGN);
	if (geteuid() != 0) {
		BEGET_LOGE("Please use root to run this program\n");
		return -1;
	}

	int ret = ParamWorkSpaceInit();
	if (ret != 0) {
		BEGET_LOGE("ParamWorkSpaceInit failed\n");
		exit(EXIT_FAILURE);
	}
	ParamServerStart();
	return 0;
}
