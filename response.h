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
#define LARGEBUF 8192

#include "util.h"

typedef unsigned char byte;

typedef struct response_buf_t {
	size_t size;
	size_t capacity;
	byte * buf;
}response_buf_t;

typedef struct {
	int respCode;
	int methodCode;
	int contentCode;
	size_t contentLength;

	char statusLine[SMALLBUF];
	char connectionLine[SMALLBUF];
	char dateLine[SMALLBUF];
	char serverLine[SMALLBUF];
	char lastModifiedLine[SMALLBUF];
	char contentLengthLine[SMALLBUF];
	char contentTypeLine[SMALLBUF];

	byte * body;
	response_buf_t * buf;
	char * objPath;
	char * redirectPath;
	FILE * objFile;

} HTTP_response_t;

typedef struct {
	byte * buf;
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
