#ifndef TRIE_UTILS_H
#define TRIE_UTILS_H

#include <stdint.h>

#define WORKSPACE_DIR "/data/service/el1/public/param_service" 
#define WORKSPACE_NAME WORKSPACE_DIR "/param.tmp"
#define WORKSPACE_SIZE (1024*1000)

typedef struct ListNode {
	uint32_t prev;
	uint32_t next;
} ListNode;

typedef struct ParamNode {
	uint8_t keyLen;
	uint8_t valueLen;
	char data[0];
} ParamNode;

typedef struct TrieNode {
	ListNode node;
	uint32_t child;
	uint32_t left;
	uint32_t right;
	uint32_t dataIndex;
	char key[0];
} TrieNode;

typedef struct TrieHeader {
	void* shareAddr;
	uint32_t trieSize;
	uint32_t paramSize;
	uint32_t rootOffest;
	uint32_t currOffest;
} TrieHeader;

int ParamWorkSpaceInit();
int SetParamtoMem(const char* key, const char* value);
int GetParamFromMem(const char* key, char* value, uint32_t len);
int WaitParam(const char* key, const char* value, uint32_t timeout);
void DumpParam();
#endif // TRIE_UTILS_H