/*
 * response.h
 *
 *  Created on: Feb 7, 2019
 *      Author: 26sra
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#define SMALLBUF 128
#define MIDBUF 1024

#include "util.h"

typedef struct response_buf_t {
	size_t size;
	size_t capacity;
	unsigned char * buf;
}response_buf_t;

typedef struct {
	int respCode;
	int methodCode;
	int contentCode;
	unsigned objPathLen;
	size_t contentLength;
	size_t size;

	unsigned char statusLine[SMALLBUF];
	unsigned char connectionLine[SMALLBUF];
	unsigned char dateLine[SMALLBUF];
	unsigned char serverLine[SMALLBUF];
	unsigned char lastModifiedLine[SMALLBUF];
	unsigned char contentLengthLine[SMALLBUF];
	unsigned char contentTypeLine[SMALLBUF];

	unsigned char * body;
	response_buf_t * buf;
	char * objPath;
	FILE * objFile;

} HTTP_response_t;

typedef struct {
	unsigned char * buf;
	ssize_t size;
} HTTP_request_t;

void
InitializeResponse(HTTP_response_t * response);

void
InitializeRequest(const ServerTCPMessage * msg,
		HTTP_request_t * request);

void
FillResponse(HTTP_request_t * request, HTTP_response_t * response);

void
FreeResponse(HTTP_response_t * response);

#endif /* RESPONSE_H_ */
