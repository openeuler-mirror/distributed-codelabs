#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <stdatomic.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

#include "trie_comm.h"
#include "le_utils.h"
#include "param_utils.h"
#include "parameter.h"
#include "securec.h"

static TrieHeader* paramWorkSpace;
static pthread_rwlock_t rwlock;
static pthread_mutex_t mtlock;
static atomic_bool cnt;
static atomic_bool waitCnt;

uint32_t trie_alloc(char* name)
{
	BEGET_ERROR_CHECK(name != NULL, return 0, "invalid name");
	uint32_t keySize = strlen(name) + 1;
	uint32_t allocSize = PARAM_ALIGN(sizeof(TrieNode) + keySize);
	uint32_t nowOffset = paramWorkSpace->currOffest;
	BEGET_ERROR_CHECK((paramWorkSpace->currOffest + allocSize) < WORKSPACE_SIZE, return 0, "no enough space to alloc trie node");
	TrieNode* item = paramWorkSpace->shareAddr + paramWorkSpace->currOffest;
	item->node.prev = 0;
	item->node.next = 0;
	item->child = 0;
	item->left = 0;
	item->right = 0;
	item->dataIndex = 0;
	int ret = memcpy_s(item->key, keySize, name, strlen(name) + 1);
	BEGET_CHECK(ret == EOK, return 0);
	++paramWorkSpace->trieSize;
	paramWorkSpace->currOffest += allocSize;
	return nowOffset;
}

uint32_t param_alloc(uint32_t size)
{
	uint32_t allocSize = PARAM_ALIGN(sizeof(ParamNode) + size);
	uint32_t nowOffset = paramWorkSpace->currOffest;
	BEGET_ERROR_CHECK((paramWorkSpace->currOffest + allocSize) < WORKSPACE_SIZE, return 0, "no enough space to alloc param node");
	ParamNode* item = paramWorkSpace->shareAddr + paramWorkSpace->currOffest;
	item->keyLen = 0;
	item->valueLen = 0;
	paramWorkSpace->currOffest += allocSize;
	++paramWorkSpace->paramSize;
	return nowOffset;
}

TrieNode* GetRootNode()
{
	BEGET_ERROR_CHECK((paramWorkSpace != NULL) && (paramWorkSpace->shareAddr != NULL), return NULL, "failed");
	return (paramWorkSpace->shareAddr + paramWorkSpace->rootOffest);
}

TrieNode* GetTrieEntry(uint32_t index)
{
	BEGET_ERROR_CHECK(index <= paramWorkSpace->currOffest, return NULL, "invalid index");
    TrieNode* entry = paramWorkSpace->shareAddr + index;
    return entry;
}

ParamNode* GetParamEntry(uint32_t index)
{
	BEGET_ERROR_CHECK(index <= paramWorkSpace->currOffest, return NULL, "invalid index");
    ParamNode* entry = paramWorkSpace->shareAddr + index;
    return entry;
}

ListNode* GetListNodeEntry(uint32_t index)
{
	BEGET_ERROR_CHECK(index <= paramWorkSpace->currOffest, return NULL, "invalid index");
	ListNode* entry = paramWorkSpace->shareAddr + index;;
	return entry;
}

TrieNode* ListNodeGetTrieEntry(ListNode* node)
{
	BEGET_ERROR_CHECK(node != NULL, return NULL, "invalid node");
    TrieNode* entry = (TrieNode*)((char*)node - offsetof(TrieNode, node));
    return entry;
}

void GetSubKey(const char* remainKey, char** subKey, uint32_t* prefixLen)
{
	BEGET_ERROR_CHECK(remainKey != NULL, return, "invalid remainKey");
	BEGET_ERROR_CHECK(subKey != NULL, return, "invalid subKey");
	*subKey = strchr(remainKey, '.');
	if (*subKey != NULL) {
		*prefixLen = *subKey - remainKey;
	} else {
		*prefixLen = strlen(remainKey);
	}
}

int CompareKey(const char* nodeKey, const char* prefixKey, uint32_t prefixKeyLen)
{
	uint32_t nodeKeyLen = strlen(nodeKey);
	if (nodeKeyLen > prefixKeyLen) {
		return -1;
	} else if (nodeKeyLen < prefixKeyLen) {
		return 1;
	}
	return strncmp(nodeKey, prefixKey, prefixKeyLen);
}

TrieNode* FindSubTrieNode(TrieNode* current, const char* remainKey, uint32_t prefixLen)
{
	if (current == NULL || remainKey == NULL)
		return NULL;
	TrieNode* subTrieNode = NULL;
	int ret = CompareKey(current->key, remainKey, prefixLen);
	if (ret == 0) {
		return current;
	} else if (ret < 0) {
		if (current->left == 0)
			return NULL;
		subTrieNode = GetTrieEntry(current->left);
	} else {
		if (current->right == 0)
			return NULL;
		subTrieNode = GetTrieEntry(current->right);
	}
	return FindSubTrieNode(subTrieNode, remainKey, prefixLen);
}

TrieNode* AddSubTrieNode(TrieNode* current, const char* remainKey, uint32_t prefixLen)
{
	if (current == NULL || remainKey == NULL)
		return NULL;
	TrieNode* subTrieNode = NULL;
	int ret = CompareKey(current->key, remainKey, prefixLen);
	if (ret == 0) {
		return current;
	} else if (ret < 0) {
		if (current->left == 0) {
			char prefixKey[PARAM_NAME_LEN_MAX] = {0};
			ret = memcpy_s(prefixKey, PARAM_NAME_LEN_MAX, remainKey, prefixLen);
			BEGET_ERROR_CHECK(ret == EOK, return NULL, "failed to memcpy to key");
			current->left = trie_alloc(prefixKey);
			BEGET_ERROR_CHECK(current->left != 0, return NULL, "failed to alloc key : %s", prefixKey);
			return GetTrieEntry(current->left);
		}
		subTrieNode = GetTrieEntry(current->left);
	} else {
		if (current->right == 0) {
			char prefixKey[PARAM_NAME_LEN_MAX] = {0};
			ret = memcpy_s(prefixKey, PARAM_NAME_LEN_MAX, remainKey, prefixLen);
			BEGET_ERROR_CHECK(ret == EOK, return NULL, "failed to memcpy to key");
			current->right = trie_alloc(prefixKey);
			BEGET_ERROR_CHECK(current->right != 0, return NULL, "failed to alloc key : %s", prefixKey);
			return GetTrieEntry(current->right);
		}
		subTrieNode = GetTrieEntry(current->right);
	}
	return AddSubTrieNode(subTrieNode, remainKey, prefixLen);
}

int CheckParamName(const char* name)
{
	BEGET_ERROR_CHECK(name != NULL, return -1, "invalid parameter name");
	size_t nameLen = strlen(name);
	if (name[0] == '.' || name[nameLen - 1] == '.')
		return -1;
	for (int i = 0; i < nameLen; ++i) {
		if (isalnum(name[i]))
			continue;
		if (name[i] == '-' || name[i] == '@' || name[i] == ':' || name[i] == '_')
			continue;
		if (name[i] == '.' && i > 0) {
			if (name[i - 1] == '.')
				return -1;
			continue;
		}
		return -1;
	}
	return 0;
}

int SetParamtoMem(const char* key, const char* value)
{
    BEGET_ERROR_CHECK((paramWorkSpace != NULL) && (paramWorkSpace->shareAddr != NULL), return -1, "invalid paramWorkSpace");
	BEGET_ERROR_CHECK((key != NULL ) && (value != NULL), return -1, "invalid key or value");
	BEGET_ERROR_CHECK((strlen(key) > 0) && (strlen(key) <= PARAM_NAME_LEN_MAX), return -1, "invalid key len");
	BEGET_ERROR_CHECK((strlen(value) > 0) && (strlen(value) <= PARAM_VALUE_LEN_MAX), return -1, "invalid value len");
	BEGET_ERROR_CHECK(CheckParamName(key) == 0, return -1, "invalid parameter name");

	TrieNode* root = GetRootNode();
	TrieNode* current = GetRootNode();
	if (root == NULL || current == NULL)
		return -1;

	char* remainKey = key;
	pthread_rwlock_wrlock(&rwlock);
	while(1) {
		char* subKey;
		uint32_t prefixLen;
		GetSubKey(remainKey, &subKey, &prefixLen);
		if (current->child != 0) {
			current = AddSubTrieNode(GetTrieEntry(current->child), remainKey, prefixLen);
			BEGET_ERROR_CHECK(current != NULL, pthread_rwlock_unlock(&rwlock); return -1, "can not AddSubTrieNode");
		} else {
			char prefixKey[PARAM_NAME_LEN_MAX] = {0};
			(void)memcpy_s(prefixKey, PARAM_NAME_LEN_MAX, remainKey, prefixLen);
			current->child = trie_alloc(prefixKey);
			BEGET_ERROR_CHECK(current->child != 0, pthread_rwlock_unlock(&rwlock); return -1, "can not alloc tire node");
			current = GetTrieEntry(current->child);
			BEGET_ERROR_CHECK(current != NULL, pthread_rwlock_unlock(&rwlock); return -1, "can not get trie entry");
		}
		if (subKey == NULL) {
			if (current->dataIndex) {
				int ret = strncmp(key, CONST_PREFIX, strlen(CONST_PREFIX)) ;
				BEGET_ERROR_CHECK(ret != 0, pthread_rwlock_unlock(&rwlock); return -1, "can not change the value of a constant parameter");
				ParamNode* saveParam = GetParamEntry(current->dataIndex);
				BEGET_ERROR_CHECK(saveParam != NULL, pthread_rwlock_unlock(&rwlock); return -1, "can not get param entry");
				(void)memcpy_s(saveParam->data + saveParam->keyLen + 1, PARAM_VALUE_LEN_MAX, value, strlen(value));
				saveParam->valueLen = strlen(value);
				break;
			}
			uint32_t allocSize = strlen(key) + PARAM_VALUE_LEN_MAX + 2;
			current->dataIndex =  param_alloc(allocSize);
			ParamNode* saveParam = GetParamEntry(current->dataIndex);
			BEGET_ERROR_CHECK((current->dataIndex != 0) && (saveParam != NULL), pthread_rwlock_unlock(&rwlock); return -1, "can not alloc param or get param entry");
			sprintf(saveParam->data, "%s=%s", key, value);
			saveParam->keyLen = strlen(key);
			saveParam->valueLen = strlen(value);

			current->node.prev = root->node.prev;
			current->node.next = (void*)(&root->node) - paramWorkSpace->shareAddr;
			ListNode* rootPrevListNode = GetListNodeEntry(root->node.prev);
			TrieNode* rootPrevTrieNode = ListNodeGetTrieEntry(rootPrevListNode);
			BEGET_ERROR_CHECK((rootPrevListNode != NULL) && (rootPrevTrieNode != NULL), pthread_rwlock_unlock(&rwlock); return -1, "can not get list entry or get trie entry");
			rootPrevTrieNode->node.next = (void*)(&current->node) - paramWorkSpace->shareAddr;
			root->node.prev = (void*)(&current->node) - paramWorkSpace->shareAddr;
			break;
		}
		remainKey = subKey + 1;
	}
	atomic_store(&cnt, 1);
	atomic_store(&waitCnt, 1);
	pthread_rwlock_unlock(&rwlock);
	return 0;
}

int GetParamFromMem(const char* key, char* value, uint32_t len)
{
    BEGET_ERROR_CHECK((paramWorkSpace != NULL) && (paramWorkSpace->shareAddr != NULL), return -1, "invalid paramWorkSpace");
	BEGET_ERROR_CHECK((key != NULL ) && (value != NULL), return -1, "invalid key or value");

	TrieNode* current = GetRootNode();
	if (current == NULL)
		return -1;

	ParamNode* paramData;
	char* remainKey = key;
	pthread_rwlock_rdlock(&rwlock);
	while (1) {
		char* subKey;
		uint32_t prefixLen;
		GetSubKey(remainKey, &subKey, &prefixLen);
		current = FindSubTrieNode(GetTrieEntry(current->child), remainKey, prefixLen);
		BEGET_ERROR_CHECK(current != NULL, pthread_rwlock_unlock(&rwlock); return -1, "can not find sub trie node : %s", key);
		if (subKey == NULL) {
			paramData = GetParamEntry(current->dataIndex);
			BEGET_ERROR_CHECK(paramData != NULL, pthread_rwlock_unlock(&rwlock); return -1, "can not get param entry");
			break;
		}
		remainKey = subKey + 1;
	}
    
	if (len > paramData->valueLen) {
		(void)memcpy_s(value, PARAM_VALUE_LEN_MAX, paramData->data + paramData->keyLen + 1, paramData->valueLen);
		value[paramData->valueLen] = '\0';
	} else {
		(void)memcpy_s(value, len, paramData->data + paramData->keyLen + 1, len);
		value[len] = '\0';
	}
	pthread_rwlock_unlock(&rwlock);
	return 0;
}

int WaitParam(const char* key, const char* value, uint32_t timeout)
{
	BEGET_ERROR_CHECK((paramWorkSpace != NULL) && (paramWorkSpace->shareAddr != NULL), return -1, "invalid paramWorkSpace");
	BEGET_ERROR_CHECK((key != NULL ) && (value != NULL), return -1, "invalid key or value");
	int ret;
	char tmp[PARAM_VALUE_LEN_MAX] = {0};
	ret = GetParamFromMem(key, tmp, sizeof(tmp));
	if (ret == 0) {
		if (strncmp(value, "*", strlen(value)) == 0) {
			return 0;
		}
		if (strlen(value) == strlen(tmp) && strncmp(value, tmp, strlen(value)) == 0) {
			return 0;
		}
		bzero(tmp, sizeof(tmp));
	}
	while (timeout != 0) {
		if (atomic_load(&waitCnt)) {
			atomic_store(&waitCnt, 0);
			ret = GetParamFromMem(key, tmp, sizeof(tmp));
			if (ret == 0) {
				if (strlen(tmp) == 1 && strncmp(value, "*", 1) == 0) {
					return 0;
				}
				if (strlen(value) == strlen(tmp) && strncmp(value, tmp, strlen(value)) == 0) {
					return 0;
				}
				bzero(tmp, sizeof(tmp));
			}
		}
		--timeout;
		sleep(1);
	}
	return -1;
}

void WritetoDisk(TrieNode* node, FILE* fp)
{
	BEGET_ERROR_CHECK(node != NULL, return, "invalid node");
    BEGET_ERROR_CHECK(fp != NULL, return, "invalid file descriptor ");

    char buf[PARAM_NAME_LEN_MAX + PARAM_VALUE_LEN_MAX];
	ParamNode* paramData = GetParamEntry(node->dataIndex);
	if (strncmp(paramData->data, PARAM_PERSIST_PREFIX, strlen(PARAM_PERSIST_PREFIX)) != 0)
		return;
	bzero(buf, sizeof(buf));
	(void)memcpy_s(buf, PARAM_NAME_LEN_MAX + PARAM_VALUE_LEN_MAX, paramData->data, paramData->keyLen + paramData->valueLen + 1);
	buf[paramData->keyLen + paramData->valueLen + 1] = '\n';
	fputs(buf, fp);
}

void DumpParam()
{
	if (atomic_load(&cnt)) {
		pthread_mutex_lock(&mtlock);
		atomic_store(&cnt, 0);
		unlink(USER_PARAM_FILE);
		FILE* fp = fopen(USER_PARAM_FILE, "a+");
		TrieNode* root = GetRootNode();
		ListNode* current = GetListNodeEntry(root->node.next);
		BEGET_ERROR_CHECK((root != NULL) && (current != NULL), pthread_mutex_unlock(&mtlock); fclose(fp); return, "can not get root node or get list entry");
		while (current != &root->node) {
			TrieNode* trienode = ListNodeGetTrieEntry(current);
			WritetoDisk(trienode, fp);
			current = GetListNodeEntry(current->next);
		}
		fclose(fp);
		pthread_mutex_unlock(&mtlock);
	}
}

void ProcessParamFile(char* fileName)
{
    BEGET_ERROR_CHECK(access(fileName, F_OK) == 0, perror("error"); return, "failed to access %s", fileName);
    FILE* fp = fopen(fileName, "r");
    BEGET_ERROR_CHECK(fp != NULL, return, "failed to open %s", fileName);
    char buf[PARAM_NAME_LEN_MAX + PARAM_VALUE_LEN_MAX];
    bzero(buf, sizeof(buf));
    while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf) - 1] = '\0';
		if (*buf == '#')
			continue;
		char *sep = buf;
		char *key = NULL;
		char *value = NULL;
		while (*sep != '\0') {
			if (*sep == '=') {
				*sep = '\0';
				value = sep + 1;
				key = buf;
				break;
			}
			++sep;
		}
        if (key) {
            SetParamtoMem(key, value);
        }	
	}
	fclose(fp);
}

void ReadFileInDir(char* dir, char* postfix)
{
	BEGET_ERROR_CHECK((dir != NULL) && (postfix != NULL), return, "invalid directory");
	DIR* pDir = opendir(dir);
	BEGET_ERROR_CHECK(pDir != NULL, return, "failed to open %s", dir);
	struct dirent* pd;
	char pdbuf[1024] = {0};

	while ((pd = readdir(pDir)) != NULL) {
		if (pd->d_type == DT_DIR)
			continue;
		char* fPostfix = strstr(pd->d_name, postfix);
		if (fPostfix == NULL)
			continue;
		if (strcmp(postfix, fPostfix) != 0)
			continue;
		bzero(pdbuf, sizeof(pdbuf));
		snprintf(pdbuf, sizeof(pdbuf), "%s/%s", dir, pd->d_name);
		ProcessParamFile(pdbuf);
	}
	closedir(pDir);
}

void LoadParam(char* dir)
{
	BEGET_ERROR_CHECK(dir != NULL, return, "invalid directory");
	struct stat st;
	if ((stat(dir, &st) == 0) && S_ISDIR(st.st_mode)) {
		ReadFileInDir(dir, ".para");
		return;
	}
}

// 定时持久化数据
void CreateParamListener()
{
	atomic_init(&cnt, 0);
	struct sigevent sigev;
	bzero(&sigev, sizeof(struct sigevent));
	sigev.sigev_notify = SIGEV_THREAD;
	sigev.sigev_notify_function = DumpParam;
	sigev.sigev_notify_attributes = NULL;

	timer_t timerId;
	if (timer_create(CLOCK_REALTIME, &sigev, &timerId) != 0) {
		perror("timer_create:");
		exit(EXIT_FAILURE);
	}

	struct itimerspec value;
	bzero(&value, sizeof(struct itimerspec));
	value.it_value.tv_sec = 1;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 1;
	value.it_interval.tv_nsec = 0;

	if (timer_settime(timerId, 0, &value, NULL) != 0) {
		perror("timer_settime:");
		exit(EXIT_FAILURE);
	}
}

void InitRootNode()
{
	BEGET_ERROR_CHECK(paramWorkSpace != NULL, return, "invalid paramWorkSpace");
	TrieNode* rootNode = paramWorkSpace->shareAddr + trie_alloc("#");
	rootNode->node.prev = (void*)(&rootNode->node) - paramWorkSpace->shareAddr;
	rootNode->node.prev = (void*)(&rootNode->node) - paramWorkSpace->shareAddr;
}

int ParamWorkSpaceInit()
{
	MakeDirRecursive(SYSTEM_PARAM_PATH, S_IRWXU | S_IRWXU | S_IRWXU | S_IROTH | S_IXOTH);
	MakeDirRecursive(USER_PARAM_PATH, S_IRWXU | S_IRWXU | S_IRWXU | S_IROTH | S_IXOTH);
	MakeDirRecursive(WORKSPACE_DIR, S_IRWXU | S_IRWXU | S_IRWXU | S_IROTH | S_IXOTH);
	pthread_rwlock_init(&rwlock, NULL);
	pthread_mutex_init(&mtlock, NULL);
	atomic_init(&waitCnt, 0);

	paramWorkSpace = (TrieHeader*)malloc(sizeof(TrieHeader));
	BEGET_ERROR_CHECK(paramWorkSpace != NULL, return -1, "failed to malloc for param workspace");
	int fd = open(WORKSPACE_NAME, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	BEGET_ERROR_CHECK(fd > 0, return -1, "failed to open %s", WORKSPACE_NAME);
	ftruncate(fd, WORKSPACE_SIZE);
	paramWorkSpace->shareAddr = mmap(NULL, WORKSPACE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	BEGET_ERROR_CHECK(paramWorkSpace->shareAddr != MAP_FAILED, return -1, "failed to create mmap");
	paramWorkSpace->rootOffest = 0;
	paramWorkSpace->currOffest = 0;
	paramWorkSpace->trieSize = 0;
	paramWorkSpace->paramSize = 0;
	InitRootNode();
    LoadParam(SYSTEM_PARAM_PATH);
	LoadParam(USER_PARAM_PATH);
	CreateParamListener();
	return 0;
}
