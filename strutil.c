/*
 * strutil.c
 *
 *  Created on: Feb 6, 2019
 *      Author: 26sra
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
	free(strCopy);
	return ret;
}

//static char *
//GetToken(const char * str, const char * delimiter,
//   unsigned n, size_t copySize, size_t size)
//{
//	char * strCopy = (char *)malloc(sizeof(char)*copySize);
//	memcpy(strCopy, str, copySize);
//	unsigned tokenCount = 0;
//	char * token = strtok(strCopy, delimiter);
//	while (token != NULL)
//	{
//		if (n==tokenCount) break;
//		token = strtok(NULL, delimiter);
//		++tokenCount;
//	}
//	char * ret = (char *)malloc(sizeof(char)*size);
//	memcpy(ret, token, size);
//	free(strCopy);
//	return ret;
//}

static void
ProcessRequestLine(char * line) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(line, " ", &tokenCount);
	for (unsigned i=0; i<tokenCount; ++i) {
		printf("%s\n", split[i]);
	}
}

int main() {
	ProcessRequestLine("GET /somedir/page.html HTTP/1.1\r\nHost: www.someschool.edu\r\n");
	printf("exiting\n");
}
