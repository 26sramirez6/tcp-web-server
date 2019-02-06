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

#define NO_MATCH -1
#define MATCH 1
#define GET 1
#define HEAD 2
#define VERSION "HTTP/1.1"
#define MAX_DIRECTORIES 20
#define ROOT "./"

typedef struct { char *k; int v; } pair_t;

static pair_t methodTable[] = {
    { "GET", GET }, { "HEAD", HEAD }
};

#define TABELIZE(A) StringEnum(A ## Table, A, sizeof(A ## Table)/sizeof(pair_t))

int StringEnum(pair_t * table, const char *key, unsigned n)
{
    for (unsigned i=0; i<n; i++) {
    	pair_t * pair = table + i*sizeof(pair_t);
        if (strcmp(pair->k, key) == 0) return pair->v;
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

static int
FileRetrieve(FILE * object, const char * root, const char * path) {
	(void)object;
	int rv = NO_MATCH;
	char * check = StringBuild(root, path);
	if( access( check, F_OK | R_OK ) != NO_MATCH ) {
	    // file exists
		printf("file %s exists\n", check);
		rv = MATCH;
	} else {
	    // file doesn't exist
		printf("file %s does not exist\n", check);
	}
	free(check);
	return rv;
}

static void
ProcessRequestLine(const char * root, const char * line) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(line, " ", &tokenCount);
	if (tokenCount != 3) {
		goto clean;
	} else if (!strcmp(split[1], VERSION)) {
		goto clean;
	}

	char * method = split[0];
	int rv = -1;
	FILE * object = NULL;
	switch (TABELIZE(method)) {
	case GET:
		rv = FileRetrieve(object, root, split[1]);
		break;
	case HEAD:
		rv = FileRetrieve(object, root, split[1]);
		break;
	case NO_MATCH:
		printf("no GET/HEAD\n");
		goto clean;
	}
	(void)rv;
	clean:
		if (tokenCount) free(split[0]);
		free(split);
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
	printf("%s\n", cwd);
	assert(cwd);
	char * root = StringBuild(cwd, "/www" );
	ProcessRequest(root, "GET /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n");
	free(cwd);
	free(root);
	printf("exiting\n");
}
