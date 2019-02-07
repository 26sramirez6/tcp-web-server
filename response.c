/*
 * strutil.c
 *
 *  Created on: Feb 6, 2019
 *      Author: 26sra
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>

#include "response.h"

#define SERVER_DEBUG

#define NO_MATCH -1
#define MATCH 1

#define CONNECTION_LINE "Connection: close"
#define LAST_MODIFIED_LINE "Last Modified: "
#define DATE_LINE "Date: "
#define SERVER_LINE "Server: "
#define CONTENT_LENGTH_LINE "Content Length: "
#define CONTENT_TYPE_LINE "Content-Type: "
#define HTTP_VERSION "HTTP/1.1"
#define CRLF "\r\n"
// table record
typedef struct { char * C1;	char * C2; int C3; } triplet_t;

// method codes
#define METHOD_GET 1
#define METHOD_HEAD 2
#define METHOD_POST 3
#define METHOD_PUT 4
#define METHOD_DELETE 5
#define METHOD_TRACE 6
#define METHOD_CONNECT 7
#define METHOD_OPTIONS 8
static triplet_t methodTable[] = {
    { "GET", "", METHOD_GET },
	{ "HEAD", "", METHOD_HEAD },
	{ "POST", "", METHOD_POST },
	{ "PUT", "", METHOD_PUT },
	{ "DELETE", "", METHOD_DELETE },
	{ "TRACE", "", METHOD_TRACE },
	{ "CONNECT", "", METHOD_CONNECT },
	{ "OPTIONS", "", METHOD_OPTIONS }
};

// response codes
#define RESPONSE_OK 200
#define RESPONSE_MOVED 301
#define RESPONSE_BAD 400
#define RESPONSE_NOT_FOUND 404
#define RESPONSE_NOT_ALLOWED 405
static triplet_t responseTable[] = {
	{ "200 OK", "", RESPONSE_OK },
	{ "301 Moved Permanently", "", RESPONSE_MOVED },
	{ "400 Bad Request", "", RESPONSE_BAD },
	{ "404 Not Found", "", RESPONSE_NOT_FOUND },
	{ "405 Method Not Allowed", "", RESPONSE_NOT_ALLOWED }
};

// content types
#define CONTENT_HTML 0
#define CONTENT_PLAIN 1
#define CONTENT_PDF 2
#define CONTENT_PNG 3
#define CONTENT_JPEG 4

#define CHUNKSIZE 1024

static triplet_t contentTable[] = {
	{ "html", ".html", CONTENT_HTML },
	{ "text/plain", ".txt", CONTENT_PLAIN },
	{ "application/pdf", ".pdf", CONTENT_PDF },
	{ "image/png", ".png", CONTENT_PNG },
	{ "image/jpeg", ".jpeg", CONTENT_JPEG }
};

#define INT_HASH(A, B) IntHash(A ## Table, A, sizeof(A ## Table)/sizeof(triplet_t), (B))
#define STR_HASH(A, B) StrHash(A ## Table, A, sizeof(A ## Table)/sizeof(triplet_t), (B))

static int
IntHash(const triplet_t * table, const char * key,
	const unsigned n, const unsigned column)
{
    for (unsigned i=0; i<n; i++) {
    	char * k = column==1 ? table[i].C1 : table[i].C2;
        if ( strcmp(k, key) == 0 )
        	return table[i]->C3;
    }
    return NO_MATCH;
}

static char *
StrHash(const triplet_t * table, const int key,
	const unsigned n, const unsigned column)
{
    for (unsigned i=0; i<n; i++) {
    	if (table[i].C3==key)
    		return column==1 ? table[i].C1 : table[i].C2;
    }
    return NULL;
}

static unsigned
GetTokenCount(const char * str, const char * delimiter)
{
	char * strCopy = strdup(str);
	unsigned tokenCount = 0;
	char * token = strtok(strCopy, delimiter);
	while (token != NULL)
	{
		token = strtok(NULL, delimiter);
		tokenCount++;
	}
	free(strCopy);
	return tokenCount;
}

static char **
StringSplit(const char * str, const char * delimiter, unsigned * n)
{
	unsigned tokenCount = GetTokenCount(str, delimiter);
	char ** ret = (char **)malloc(sizeof(char *)*tokenCount);
	char * strCopy = strdup(str);
	char * token = strtok(strCopy, delimiter);
	for (unsigned i=0; i<tokenCount; i++)
	{
		ret[i] = token;
		token = strtok(NULL, delimiter);
	}
	*n = tokenCount;
	return ret;
}

static unsigned
StringBuilder(unsigned char * to, const unsigned char * from) {
	size_t s1L = strlen((const char *)to);
	size_t s2L = strlen((const char *)from);
	to = (unsigned char *)realloc(to, s1L+s2L+1);
	assert(to);
	to[s1L+s2L] = 0;
	for (unsigned i=0;i<s2L;++i)
		to[s1L+i] = from[i];
	return s1L+s2L;
}

void
FileRetrieve(HTTP_response_t * response) {
	struct stat fileStats;
	stat(response->objPath, &fileStats);
	strftime(response->lastModifiedLine, SMALLBUF,
		"%a, %d %b %Y %H:%M:%S %Z", &fileStats);
	response->objFile = fopen(response->objPath, "rb");
	assert(response->objFile);
}

void
ChunkRead(HTTP_response_t * response) {
	assert(response->objFile);
	assert(response->body==NULL);
	size_t bytesRead = -1;
	size_t totalBytesRead = 0;
	int i;
	for (i=1; bytesRead>0; ++i) {
		response->body = (unsigned char *)realloc(response->body, CHUNKSIZE*i);
		bytesRead = fread(response->body, 1, CHUNKSIZE, response->objFile);
		totalBytesRead += bytesRead;
	}
	if (totalBytesRead<CHUNKSIZE*i)
		response->contentLength[totalBytesRead] = 0;
	response->contentLength = totalBytesRead;
}

void
FillContentType(HTTP_response_t * response) {
	char * content = strrchr(response->objPath, '.') + 1;
	response->contentCode = INT_HASH(content, 2);
	response->contentTypeLine = contentTable[response->contentCode].C1;
}

#define FILE_EXISTS(path) !(access( (path) , F_OK | R_OK ))

void
ProcessValidMethod(HTTP_response_t * response) {
	unsigned char * body = NULL;
	if( FILE_EXISTS(response->objPath) ) {
		response->respCode = RESPONSE_OK;
		FileRetrieve(response);
		if ( response->methodCode == METHOD_GET ) {
			ChunkRead(response);
		} else {
			response->contentLength = 0;
		}
		response->body = body;
		FillContentType(response);
	} else {
		response->respCode = RESPONSE_NOT_FOUND;
	}

#ifdef SERVER_DEBUG
	if (response->respCode==RESPONSE_OK) {
		printf("object %s found\n", response->objPath);
	} else {
		printf("object %s not found\n", response->objPath);
	}
#endif
}

void
FillResponseBuf(HTTP_response_t * resp) {
	assert(resp->buf==NULL);
	// status line
	StringBuilder(resp->buf, HTTP_VERSION);
	StringBuilder(resp->buf, " ");
	int response = resp->respCode;
	StringBuilder(resp->buf, STR_HASH(response, 1));
	StringBuilder(resp->buf, CRLF);

	if (response != RESPONSE_OK) {
		resp->size = strlen(resp->buf);
	}

	// connection type line
	StringBuilder(resp->buf, CONNECTION_LINE);
	StringBuilder(resp->buf, CRLF);
	// date line
	StringBuilder(resp->buf, DATE_LINE);
	StringBuilder(resp->buf, resp->dateLine);
	StringBuilder(resp->buf, CRLF);
	// server line
	StringBuilder(resp->buf, SERVER_LINE);
	StringBuilder(resp->buf, resp->serverLine);
	StringBuilder(resp->buf, CRLF);
	// last modified line
	StringBuilder(resp->buf, LAST_MODIFIED_LINE);
	StringBuilder(resp->buf, resp->lastModifiedLine);
	StringBuilder(resp->buf, CRLF);
	// content length line
	StringBuilder(resp->buf, CONTENT_LENGTH_LINE);
	const int n = snprintf(NULL, 0, "%zu", resp->contentLength);
	assert(n > 0);
	char contentLength[n+1];
	const int c = snprintf(contentLength, n+1, "%zu", resp->contentLength);
	assert(contentLength[n] == 0);
	assert(c == n);
	StringBuilder(resp->buf, contentLength);
	StringBuilder(resp->buf, CRLF);
	// content type line
	StringBuilder(resp->buf, CONTENT_TYPE_LINE);
	StringBuilder(resp->buf, resp->contentTypeLine);
	StringBuilder(resp->buf, CRLF);
	// final crlf
	resp->size = StringBuilder(resp->buf, CRLF);
	// copy the data over finally
	resp->buf = realloc(resp->buf, resp->size+resp->contentLength+1);
	resp->buf[resp->size+resp->contentLength] = 0;
	memcpy(resp->buf[resp->contentLength], resp->body, resp->contentLength);
	resp->size += resp->contentLength;
}

static
ProcessRequestLine(const char * requestLine, HTTP_response_t * response) {
	unsigned tokenCount = 0;
	char * body = NULL;
	char ** split = StringSplit(requestLine, " ", &tokenCount);
	if (tokenCount != 3) {
		response->respCode = RESPONSE_BAD;
		goto clean;
	} else if (!strcmp(split[1], HTTP_VERSION)) {
		response->respCode = RESPONSE_BAD;
		goto clean;
	}
	response->objPath = StringBuilder(response->objPath, split[1]);
	char * method = split[0];
	response->methodCode = INT_HASH(method, 1);
	switch ( response->methodCode ) {
	case METHOD_GET:
	case METHOD_HEAD:
		ProcessValidMethod(response);
		break;
	case METHOD_POST:
	case METHOD_PUT:
	case METHOD_DELETE:
	case METHOD_TRACE:
	case METHOD_CONNECT:
	case METHOD_OPTIONS:
		response->respCode = RESPONSE_NOT_ALLOWED;
		printf("not allowed\n");
		break;
	case NO_MATCH:
		response->respCode = RESPONSE_BAD;
		printf("bad request\n");
	}

	FillResponseBuf(response);

	clean:
		if (tokenCount) free(split[0]);
		free(split);
}

static void
ProcessRequest(const char * root, const char * request,
		HTTP_response_t * response) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(request, "\r\n", &tokenCount);
	ProcessRequestLine(root, split[0], response);
	if (tokenCount) free(split[0]);
	free(split);
}

void
InitializeResponse(HTTP_response_t * response) {
	int rv = 0;
	char connLine[] = CONNECTION_LINE;
	memcpy(response->connectionLine, connLine, sizeof(connLine));
	memset(response->contentLengthLine, 0, SMALLBUF);
	memset(response->contentTypeLine, 0, SMALLBUF);
	memset(response->lastModifiedLine, 0, SMALLBUF);
	memset(response->serverLine, 0, SMALLBUF);
	memset(response->statusLine, 0, SMALLBUF);
	memset(response->dateLine, 0, SMALLBUF);
	response->body = NULL;
	response->contentCode = NO_MATCH;
	response->contentLength = 0;
	response->methodCode = NO_MATCH;
	response->respCode = NO_MATCH;
	response->objFile = NULL;
	time_t now = time(0);
	struct tm* timeInfo = gmtime(&now);
	rv = strftime(response->dateLine, SMALLBUF,
			"%a, %d %b %Y %H:%M:%S %Z", timeInfo);
	assert(rv);
	rv = gethostname(response->serverLine, SMALLBUF);
	assert(rv==0);

	response->objPath = getcwd(NULL, 0);
	assert(response->objPath);
	response->objPath = StringBuilder(response->objPath, "/www");
}

void
InitializeRequest(const ServerTCPMessage * msg,
		HTTP_request_t * request) {
	request->buf = msg->buf;
	request->size = msg->bytesOut;
}

void
FillResponse(HTTP_request_t * request, HTTP_response_t * response) {
	unsigned tokenCount = 0;
	char ** split = StringSplit((char *)request->buf, "\r\n", &tokenCount);
	ProcessRequestLine(split[0], response);
	if (tokenCount) free(split[0]);
	free(split);
}

int main() {
	char * cwd = getcwd(NULL, 0);
	assert(cwd);
	char * root = StringBuild(cwd, "/www");
	char * req = "OPTIONS /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	InitializeResponse(&response);

	ProcessRequest(root, req, &response);
	free(cwd);
	free(root);
	printf("exiting\n");
};
