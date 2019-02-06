/*
 * web_server.c
 *
 *  Created on: Feb 5, 2019
 *      Author: sramirez266
 */


#include "util.h"
#include "signal.h"
#define BACKLOG 10 // number of pending connections queue will hold
#define GOOD "HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n"

void ServerCallback(ServerTCPMessage * serverMsg) {
	unsigned char buf[] = GOOD;
	memcpy(serverMsg->buf, buf, sizeof(buf));
	serverMsg->bytesOut = sizeof(buf);
#ifdef SERVER_DEBUG
	printf("ServerCallback::Server sending %zd bytes to %s:\n%s\n",
				serverMsg->bytesOut, serverMsg->clientName,
				serverMsg->buf);
#endif
	send(serverMsg->tcpFd, serverMsg->buf,
			serverMsg->bytesOut, 0);
}

//void Response(char * buf, const size_t bytes) {
//	assert(snprintf(buf, bytes, "%s%s\n", GOOD, CONNECTION)==bytes-1);
//}

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
