#include "beget_ext.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF 1024

void SetNoBlock(int fd)
{
        int status = fcntl(fd, F_GETFL);
        if (status < 0) {
                return;
        }

        status |= O_NONBLOCK;
        status = fcntl(fd, F_SETFL, status);
        if (status < 0) {
                return;
        }
        return;
}

void SetCloseExec(int fd)
{
    int option = fcntl(fd, F_GETFD);
    if (option < 0) {
        return;
    }

    option = option | FD_CLOEXEC;
    option = fcntl(fd, F_SETFD, option);
    if (option < 0) {
        return;
    }
    return;
}

int Makedir_(char* dir, mode_t mode)
{
        BEGET_ERROR_CHECK(dir != NULL, return -1, "invalid dri");
        if (access(dir, F_OK) == 0)
                return 0;
        
        return mkdir(dir, mode);
}

int MakeDirRecursive(char* dir, mode_t mode)
{
        if (access(dir, F_OK) == 0)
                return -1;
        char dirbuf[MAX_BUF] = {0};
        char* p = dir;
        char* slash;
        int gap;
        while ((slash = strchr(p, '/')) != NULL) {
                p = slash + 1;
                slash = strchr(p, '/');
                if (slash == NULL)
                        gap = strlen(dir);
                else
                        gap = slash - dir;

                (void)memcpy_s(dirbuf, MAX_BUF, dir, gap);
                int ret = Makedir_(dirbuf, mode);
                BEGET_ERROR_CHECK(ret == 0, return ret, "failed to create %s", dirbuf);
        }
        return 0;
}

