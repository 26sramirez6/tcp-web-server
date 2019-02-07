/*
 * web_server.c
 *
 *  Created on: Feb 5, 2019
 *      Author: sramirez266
 */

#include "response.h"
#include "util.h"

#define BACKLOG 10 // number of pending connections queue will hold

static void ServerCallback(const ServerTCPMessage * serverMsg) {
	HTTP_response_t response;
	HTTP_request_t request;
	InitializeRequest(serverMsg, &request);
	InitializeResponse(&response);
	FillResponse(&request, &response);

#ifdef SERVER_DEBUG
	printf("ServerCallback::Server sending %zd bytes to %s:\n%s\n",
			response->size, serverMsg->clientName, response->buf);
#endif
	send(serverMsg->tcpFd, response->buf, response->size, 0);
	FreeResponse(&response);
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
