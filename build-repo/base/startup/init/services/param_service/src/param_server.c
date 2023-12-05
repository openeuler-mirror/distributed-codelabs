#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "beget_ext.h"
#include "param_server.h"
#include "param_request.h"
#include "param_utils.h"
#include "trie_comm.h"
#include "securec.h"

void HandleEvent(struct EventArgs* args)
{
	int clientFd = args->clientFd;
	int epollFd = args->epollFd;
	struct ParamRequestMsg* pmsg = (struct ParamRequestMsg*)malloc(sizeof(struct ParamRequestMsg) + PARAM_VALUE_LEN_MAX);
	BEGET_ERROR_CHECK(pmsg != NULL, return, "failed to malloc ParamRequestMsg");
	bzero(pmsg, sizeof(struct ParamRequestMsg) + PARAM_VALUE_LEN_MAX);
	pmsg->datasize = PARAM_VALUE_LEN_MAX;
	int status = SOCK_CONNECTED;
	while (1) {
		int ret = recv(clientFd, pmsg, sizeof(struct ParamRequestMsg) + pmsg->datasize, 0);
		if (ret == 0) {
			status = SOCK_DISCONNECTED;
			break;
		} else if (ret < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
				continue;
			}
			status = SOCK_UNKNOWN;
			break;
		} else {
			break;
		}
	}

	if (status != SOCK_CONNECTED) {
		epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
		free(pmsg);
		return;
	}

	int ret;
	struct ParamRespMsg* respmsg = (struct ParamRespMsg*)malloc(sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX);
	BEGET_ERROR_CHECK(respmsg != NULL, free(pmsg);return, "Failed to malloc ParamRespMsg");
	bzero(respmsg, sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX);
	switch(pmsg->type) {
		case SET_PARAMETER: {
			ret = SetParamtoMem(pmsg->key, pmsg->data);
			respmsg->flag = ret;
			break;
		}
		case GET_PARAMETER: {
			if (pmsg->datasize > PARAM_VALUE_LEN_MAX) {
				pmsg->datasize = PARAM_VALUE_LEN_MAX;
			}
			ret = GetParamFromMem(pmsg->key, respmsg->data, pmsg->datasize);
			respmsg->flag = ret;
			if (ret == 0) {
				respmsg->datasize = strlen(respmsg->data);
			}
			break;
		}
		case WAIT_PARAMETER: {
			ret = WaitParam(pmsg->key, pmsg->data, pmsg->timeout);
			respmsg->flag = ret;
			break; 
		}
		default:
			respmsg->flag = -1;
			break;
	}

	ret = send(clientFd, respmsg, sizeof(struct ParamRespMsg) + PARAM_VALUE_LEN_MAX, 0);
	if (ret < 0) {
		BEGET_LOGE("Failed to send data to : %d\n", clientFd);
	}
	free(pmsg);
	free(respmsg);
}

int CtlAdd(int epollfd, int fd, uint32_t event)
{
	struct epoll_event ev = {
		.data.fd = fd,
		.events = event,
	};
	int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	BEGET_ERROR_CHECK(ret == 0, return -1, "failed to add epoll_ctl fd %d. errno [%d]", fd, errno);
	return 0;
}

void StartEpoll(int listenfd)
{
    int epollfd = epoll_create(LOOP_MAX_SOCKET);
	BEGET_ERROR_CHECK(epollfd > 0, return, "failed to create epoll. errno [%d]", errno);

	int ret = CtlAdd(epollfd, listenfd, EPOLLIN);
	BEGET_ERROR_CHECK(ret == 0, close(epollfd); return, "failed to CtlAdd");

	struct epoll_event *events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * LOOP_MAX_SOCKET);
	BEGET_ERROR_CHECK(events != NULL, close(epollfd); return, "failed to alloc memory for epoll_event");

	while(1) {
		int number = epoll_wait(epollfd, events, LOOP_MAX_SOCKET, -1);
		for (int index = 0; index < number; ++index) {
			int fd_ = events[index].data.fd;
			if (fd_ == listenfd) {
				struct sockaddr_un clientAddr;
				socklen_t addrlen = sizeof(clientAddr);
				bzero(&clientAddr, addrlen);
				int clientfd = accept(listenfd, (struct sockaddr*)&clientAddr, &addrlen);
				BEGET_ERROR_CHECK(clientfd >= 0, close(epollfd); return, "failed to accept socket");
				SetNoBlock(clientfd);
				SetCloseExec(clientfd);
				ret = CtlAdd(epollfd, clientfd, EPOLLIN | EPOLLET | EPOLLONESHOT);
				BEGET_ERROR_CHECK(ret == 0, continue, "failed to CtlAdd");
			} else {
				pthread_t threadId;
				struct EventArgs args = {epollfd, fd_};
				ret = pthread_create(&threadId, NULL, (void*)HandleEvent, (void*)&args);
				BEGET_ERROR_CHECK(ret == 0, continue, "faild to create pthread to handle parameter event");
			}
		}
	}
	close(epollfd);
	free(events);
}

int CreateSocket()
{
	unlink(PIPE_NAME);
	int listenfd = socket(PF_UNIX, SOCK_STREAM, 0);
	BEGET_ERROR_CHECK(listenfd > 0, return -1, "failed to create socket. errno [%d]", errno);

	struct sockaddr_un serverAddr;
	
	(void)memset_s(&serverAddr, sizeof(serverAddr), 0, sizeof(serverAddr));
	serverAddr.sun_family = AF_UNIX;
	strncpy(serverAddr.sun_path, PIPE_NAME, sizeof(serverAddr.sun_path));

	uint32_t size = offsetof(struct sockaddr_un, sun_path) + strlen(PIPE_NAME);
	int ret = bind(listenfd, (struct sockaddr*)&serverAddr, size);
	BEGET_ERROR_CHECK(ret >= 0, close(listenfd); return -1, "failed to bind socket. errno [%d]", errno);

	SetNoBlock(listenfd);
	SetCloseExec(listenfd);
	ret = listen(listenfd, LOOP_MAX_CLIENT);
	BEGET_ERROR_CHECK(ret >= 0, close(listenfd); return -1, "failed to listen socket. errno [%d]", errno);
	ret = chmod(PIPE_NAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	BEGET_ERROR_CHECK(ret == 0, close(listenfd); return -1, "failed to chmod %s. errno [%d]", PIPE_NAME, errno);

	return listenfd;
}

void ParamServerStart()
{
	MakeDirRecursive(PIPE_PATH, S_IRWXU | S_IRWXU | S_IRWXU | S_IROTH | S_IXOTH);
	int listenfd = CreateSocket();
	StartEpoll(listenfd);
}
