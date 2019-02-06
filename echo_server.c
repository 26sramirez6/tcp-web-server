/*
 * echo_server.c
 *
 *  Created on: Jan 16, 2019
 *      Author: 26sra
 */

#include "util.h"
#define BACKLOG 10 // number of pending connections queue will hold

void ServerCallback(ServerTCPMessage * serverMsg) {

#ifdef SERVER_DEBUG
	printf("Server sending %zd bytes to %s:%s\n",
				serverMsg->bytesOut, serverMsg->clientName,
				serverMsg->buf);
#endif
	printf("%s", serverMsg->buf);
	send(serverMsg->tcpFd, serverMsg->buf,
			serverMsg->bytesOut, 0);
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
