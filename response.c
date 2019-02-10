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

//#define NDEBUG
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
	const unsigned n, const unsigned column) {
    for (unsigned i=0; i<n; i++) {
    	char * k = column==1 ? table[i].C1 : table[i].C2;
        if ( strcmp(k, key) == 0 )
        	return table[i]->C3;
    }
    return NO_MATCH;
}

static char *
StrHash(const triplet_t * table, const int key,
	const unsigned n, const unsigned column) {
    for (unsigned i=0; i<n; i++) {
    	if (table[i].C3==key)
    		return column==1 ? table[i].C1 : table[i].C2;
    }
    return NULL;
}

static unsigned
GetTokenCount(const char * str, const char * delimiter) {
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
StringSplit(const char * str, const char * delimiter, unsigned * n) {
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

static response_buf_t *
ResponseBufCreate() {
	response_buf_t * ret = malloc(sizeof(response_buf_t));
	ret->capacity = MIDBUF;
	ret->size  = 0;
	ret->buf = (char *)malloc(MIDBUF);
	return ret;
}

static void
ResponseBufBuilder(response_buf_t * to, const char * from, size_t n) {
	assert(to->size<=to->capacity);
	size_t len = 0;
	if (!n) {
		len = n;
	} else {
		len = strlen((const char *)from);
	}
	// adjust size of the buffer according
	// to however much needs to be added
	while (to->capacity < len + to->size) {
		char * copy = (char *)malloc(to->buf, to->capacity*2);
		assert(copy);
		memcpy(copy, to->buf, to->capacity);
		free(to->buf);
		to->buf = copy;
		to->capacity = to->capacity*2;
	}

	// NE more explicit. the LHS is either equal or
	// less than capacity based on the loop above.
	if (to->size+len!=to->capacity) to->buf[to->size+len] = 0;
	for (unsigned i=0;i<len;++i) to->buf[to->size+i] = from[i];
	to->size += len;
}

static void
FreeResponseBuf(response_buf_t * rb) { free(rb->buf); free(rb); }


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
		response->body = (char *)realloc(response->body, CHUNKSIZE*i);
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
	char * body = NULL;
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
	ResponseBufBuilder(resp->buf, HTTP_VERSION);
	ResponseBufBuilder(resp->buf, " ");
	int response = resp->respCode;
	ResponseBufBuilder(resp->buf, STR_HASH(response, 1));
	ResponseBufBuilder(resp->buf, CRLF);

	if (response != RESPONSE_OK) {
		resp->size = strlen(resp->buf);
	}

	// connection type line
	ResponseBufBuilder(resp->buf, CONNECTION_LINE, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// date line
	ResponseBufBuilder(resp->buf, DATE_LINE, 0);
	ResponseBufBuilder(resp->buf, resp->dateLine, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// server line
	ResponseBufBuilder(resp->buf, SERVER_LINE, 0);
	ResponseBufBuilder(resp->buf, resp->serverLine, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// last modified line
	ResponseBufBuilder(resp->buf, LAST_MODIFIED_LINE, 0);
	ResponseBufBuilder(resp->buf, resp->lastModifiedLine, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// content length line
	ResponseBufBuilder(resp->buf, CONTENT_LENGTH_LINE, 0);
	const int n = snprintf(NULL, 0, "%zu", resp->contentLength);
	assert(n > 0);
	char contentLength[n+1];
	const int c = snprintf(contentLength, n+1, "%zu", resp->contentLength);
	assert(contentLength[n] == 0);
	assert(c == n);
	ResponseBufBuilder(resp->buf, contentLength, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// content type line
	ResponseBufBuilder(resp->buf, CONTENT_TYPE_LINE, 0);
	ResponseBufBuilder(resp->buf, resp->contentTypeLine, 0);
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// final crlf before data
	ResponseBufBuilder(resp->buf, CRLF, 0);
	// copy the data over finally
	ResponseBufBuilder(resp->buf, resp->body, resp->contentLength);
}

static void
ProcessRequest(const char * requestLine, HTTP_response_t * response) {
}


void
InitializeResponse(HTTP_response_t * response) {
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
	response->buf = ResponseBufCreate();

	time_t now = time(0);
	struct tm* timeInfo = gmtime(&now);
	size_t rv1 = 0;
	rv1 = strftime((char *)response->dateLine, SMALLBUF,
			"%a, %d %b %Y %H:%M:%S %Z", timeInfo);
	assert(rv1);
	int rv2 = 0;
	rv2 = gethostname((char *)response->serverLine, SMALLBUF);
	assert(rv2!=-1);

	response->objPath = getcwd(NULL, 0);
	assert(response->objPath);
	response->objPath = strcat(response->objPath, "/www");
}

void
InitializeRequest(const ServerTCPMessage * msg,
		HTTP_request_t * request) {
	request->buf = msg->buf;
	request->size = msg->bytesOut;
}

void
FillResponse(HTTP_request_t * request, HTTP_response_t * response) {
	char * body = NULL;
	unsigned reqTokens = 0;
	unsigned reqHeadTokens = 0;
	char ** reqSplit = StringSplit((char *)request->buf, "\r\n", &reqTokens);
	char ** reqHeadSplit = StringSplit(reqSplit[0], " ", &reqHeadTokens);
	if (reqHeadTokens != 3) {
		response->respCode = RESPONSE_BAD;
		goto clean;
	} else if (!strcmp(reqHeadSplit[1], HTTP_VERSION)) {
		response->respCode = RESPONSE_BAD;
		goto clean;
	}
	response->objPath = strcat(response->objPath, reqHeadSplit[1]);
	char * method = reqHeadSplit[0];
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
		if (reqTokens) free(reqSplit[0]);
		free(reqSplit);
		if (reqHeadTokens) free(reqHeadSplit[0]);
		free(reqHeadSplit);
}

void
FreeResponse(HTTP_response_t * response) {
	if (response->buf!=NULL) FreeResponseBuf(response->buf);
	if (response->objFile!=NULL) fclose(response->objFile);
	if (response->objPath!=NULL) free(response->objPath);
	if (response->body!=NULL) free(response->body);
}

int main() {
	char * cwd = getcwd(NULL, 0);
	assert(cwd);
	char * root = strcat(cwd, "/www");
	char * req = "OPTIONS /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	InitializeResponse(&response);

	ProcessRequest(root, req, &response);
	free(cwd);
	free(root);
	printf("exiting\n");
}
