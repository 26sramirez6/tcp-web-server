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


#define SERVER_DEBUG
#define NO_MATCH -1
#define MATCH 1

#define VERSION "HTTP/1.1"
#define MAX_DIRECTORIES 20
#define ROOT "./"

typedef struct { char *k; int v; } kv_t;
typedef struct { int v; char *k;  } vk_t;
typedef struct { char * C1;	char * C2; unsigned C3; } triplet_t;

// methods
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
//static vk_t responseTable[] = {
//	{ "OK", RESPONSE_OK },
//	{ "Moved Permanently", RESPONSE_MOVED },
//	{ "Bad Request", RESPONSE_BAD },
//	{ "Not Found", RESPONSE_NOT_FOUND },
//	{ "Method Not Allowed", RESPONSE_NOT_ALLOWED }
//};

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

#define HASH_LOOKUP(A, B) StringEnum(A ## Table, A, sizeof(A ## Table)/sizeof(triplet_t), (B))

int
StringEnum(const triplet_t * table, const char * key,
	const unsigned n, const unsigned column)
{
    for (unsigned i=0; i<n; i++) {
    	char * k = column==1 ? table[i].C1 : table[i].C2;
        if ( strcmp(k, key) == 0 )
        	return table.C3;
    }
    return NO_MATCH;
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

static char *
StringBuild(const char * s1, const char * s2) {
	size_t s1L = strlen(s1);
	size_t s2L = strlen(s2);
	char * ret = (char *)malloc(s1L+s2L+1);
	assert(ret);
	ret[s1L+s2L] = 0;
	for (unsigned i=0;i<s1L;++i) ret[i] = s1[i];
	for (unsigned i=0;i<s2L;++i) ret[s1L+i] = s2[i];
	return ret;
}

void
ConstructResponse(const unsigned responseCode, const unsigned methodCode,
		const char * root, const char * requestLine) {

	switch (responseCode) {
	case RESPONSE_OK:

	case RESPONSE_MOVED:
	case RESPONSE_BAD:
	case RESPONSE_NOT_FOUND:
	case RESPONSE_NOT_ALLOWED:
	}
}

FILE *
FileRetrieve(const char * path) {
	FILE * object = fopen(path, "rb");
	assert(object);
	return object;
}

unsigned char *
ChunkRead(FILE * obj) {
	assert(obj);
	size_t bytesRead = -1;
	size_t totalBytesRead = 0;
	unsigned char * ret = NULL;
	int i;
	for (i=1; bytesRead>0; ++i) {
		ret = (unsigned char *)realloc(ret, CHUNKSIZE*i);
		bytesRead = fread(ret, 1, CHUNKSIZE, obj);
		totalBytesRead += bytesRead;
	}
	if (totalBytesRead<CHUNKSIZE*i) ret[totalBytesRead] = 0;
	return ret;
}

typedef struct {
	char[BUFLEN] contentType;
}HTTP_response_t;

void SetContentType(const char * path, HTTP_response_t * response) {
	char * content = strrchr(path, '.') + 1;
	int contentCode = HASH_LOOKUP(content, 2);
	switch (  )
	case CONTENT_HTML:
		response->contentType = contentTable[contentCode].C1;
	case CONTENT_PLAIN:

		break;
	case CONTENT_PDF:
	case CONTENT_PNG:
	case CONTENT_JPEG:

		break;
}

#define FILE_EXISTS(path) !(access( (path) , F_OK | R_OK ))

void
ProcessValidMethod(const char * path, const int methodCode, int * rv) {
	FILE * object = NULL;
	unsigned char * body = NULL;
	if( FILE_EXISTS(path) ) {
		rv = RESPONSE_OK;
		object = FileRetrieve(path);
		if ( methodCode == METHOD_GET ) {
			body = ChunkRead(object);
		} else { // method is HEAD
			// malloc anyways to free later
			body = (unsigned char *) malloc(1);
			body[0] = 0;
		}
	} else {
		rv = RESPONSE_NOT_FOUND;
	}

#ifdef SERVER_DEBUG
	if (rv==RESPONSE_OK) {
		printf("object %s found\n", path);
	} else {
		printf("object %s not found\n", path);
	}
#endif
}

static int
ProcessRequestLine(const char * root, const char * requestLine) {
	unsigned tokenCount = 0;
	char * path = NULL;
	char * body = NULL;
	FILE * object = NULL;
	char ** split = StringSplit(requestLine, " ", &tokenCount);
	if (tokenCount != 3) {
		goto clean;
	} else if (!strcmp(split[1], VERSION)) {
		goto clean;
	}
	path = StringBuild(root, split[1]);
	char * method = split[0];
	int rv = RESPONSE_BAD;
	int methodCode = HASH_LOOKUP(method, 1);
	switch ( methodCode ) {
	case METHOD_GET:
	case METHOD_HEAD:
		ProcessValidMethod(path, methodCode, &rv);
		break;
	case METHOD_POST:
	case METHOD_PUT:
	case METHOD_DELETE:
	case METHOD_TRACE:
	case METHOD_CONNECT:
	case METHOD_OPTIONS:
		rv = RESPONSE_NOT_ALLOWED;
		printf("not allowed\n");
		break;
	case NO_MATCH:
		rv = RESPONSE_BAD;
		printf("bad request\n");
		goto clean;
	}
	(void)rv;

	clean:
		if (tokenCount) free(split[0]);
		if (path) free(path);
		if (object) fclose(object);
		free(split);

	return rv;
}

static void
ProcessRequest(const char * root, const char * request) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(request, "\r\n", &tokenCount);
	ProcessRequestLine(root, split[0]);
	if (tokenCount) free(split[0]);
	free(split);
}

int main() {
	char * cwd = getcwd(NULL, 0);
	assert(cwd);
	char * root = StringBuild(cwd, "/www" );
	ProcessRequest(root, "OPTIONS /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n");
	free(cwd);
	free(root);
	printf("exiting\n");
}
