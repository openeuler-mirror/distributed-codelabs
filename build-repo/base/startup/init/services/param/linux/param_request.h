#ifndef PARAM_REQUEST_H
#define PARAM_REQUEST_H

#include "parameter.h"

typedef struct ParamRequestMsg {
	uint32_t type;
	uint32_t datasize;
	uint32_t timeout;
	char key[PARAM_NAME_LEN_MAX];
	char data[0];
} ParamRequestMsg;

typedef struct ParamRespMsg {
	uint32_t flag;
	uint32_t datasize;
	char data[0];
} ParamRespMsg;

enum {
	SET_PARAMETER = 0,
	GET_PARAMETER,
	WAIT_PARAMETER,
};

#endif // PARAM_REQUEST_H