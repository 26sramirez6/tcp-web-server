/*
 * response.h
 *
 *  Created on: Feb 7, 2019
 *      Author: 26sra
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include "vararray.h"
#include "util.h"

#define SMALLBUF 128
#define MIDBUF 1024
#define LARGEBUF 8192
#define CHUNKSIZE 1024

#define SERVER_DEBUG

#define NO_MATCH -1
#define MATCH 1
#define REDIRECT_PATH "/redirect.defs"
#define ROOT "/www"
#define CONNECTION_LINE "Connection: close"
#define LAST_MODIFIED_LINE "Last Modified: "
#define DATE_LINE "Date: "
#define SERVER_LINE "Server: "
#define CONTENT_LENGTH_LINE "Content Length: "
#define CONTENT_TYPE_LINE "Content-Type: "
#define LOCATION_LINE "Location: "
#define HTTP_VERSION "HTTP/1.1"
#define CRLF "\r\n"

// method codes
#define METHOD_GET 1
#define METHOD_HEAD 2
#define METHOD_POST 3
#define METHOD_PUT 4
#define METHOD_DELETE 5
#define METHOD_TRACE 6
#define METHOD_CONNECT 7
#define METHOD_OPTIONS 8

// response codes
#define RESPONSE_OK 200
#define RESPONSE_MOVED 301
#define RESPONSE_BAD 400
#define RESPONSE_NOT_FOUND 404
#define RESPONSE_NOT_ALLOWED 405

// content types
#define CONTENT_HTML 0
#define CONTENT_PLAIN 1
#define CONTENT_PDF 2
#define CONTENT_PNG 3
#define CONTENT_JPEG 4


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

	char * objRequested;
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

void
FreeRedirects();

#endif /* RESPONSE_H_ */
