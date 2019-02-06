/*
 * web_server.c
 *
 *  Created on: Feb 5, 2019
 *      Author: sramirez266
 */

#include <ftw.h>
#include "util.h"

#define BACKLOG 10 // number of pending connections queue will hold
#define GOOD "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n"

static void ExtractHTMLObject(char * buf) {

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
	int tokenCount = GetTokenCount(str, delimiter);
	char ** ret = (char **)malloc(sizeof(char *)*tokenCount);
	char * strCopy = strdup(str);
	char * token = strtok(strCopy, delimiter);
	for (unsigned i = 0; i<tokenCount; i++)
	{
		ret[i] = token;
		token = strtok(NULL, delimiter);
	}
	*n = tokenCount;
	free(strCopy);
	return ret;
}

char *
GetToken(const char * str, const char * delimiter,
   unsigned n, size_t copySize, size_t size)
{
	char * strCopy = (char *)malloc(sizeof(char)*copySize);
	memcpy(strCopy, str, copySize);
	unsigned tokenCount = 0;
	char * token = strtok(strCopy, delimiter);
	while (token != NULL)
	{
		if (n==tokenCount) break;
		token = strtok(NULL, delimiter);
		++tokenCount;
	}
	char * ret = (char *)malloc(sizeof(char)*size);
	memcpy(ret, token, size);
	free(strCopy);
	return ret;
}

static void
ProcessRequestLine(char * line) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(line, " ", &tokenCount);
	for (int i=0; i<tokenCount; ++i) {
		printf("%s\n", split[i]);
	}
}

static void
ProcessHTTPRequest(const ServerTCPMessage * serverMsg) {
	unsigned tokenCount = 0;
	char ** split = StringSplit(serverMsg->buf, "\r\n", &tokenCount);
	ProcessRequestLine(split[0]);
	for (int i=0;i<tokenCount;++i) free(split[i]);
}

static void ServerCallback(const ServerTCPMessage * serverMsg) {
	ProcessRequest(serverMsg);
	unsigned char buf[] = GOOD;
#ifdef SERVER_DEBUG
	printf("ServerCallback::Server sending %zd bytes to %s:\n%s\n",
			sizeof(buf), serverMsg->clientName, buf);
#endif
	send(serverMsg->tcpFd, buf, sizeof(buf), 0);
}


int main(int argc, char ** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: echo_server <port>\n");
		exit(EXIT_FAILURE);
	}

	// listen on a socket and return the descriptor
	int listeningFd = ServerInitTCP(argv[1], BACKLOG);
	// accept connections and perform callback in a while loop
	ServerSpinTCP(listeningFd, ServerCallback);
}
