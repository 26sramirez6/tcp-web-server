/*
 * echo_client.c
 *
 *  Created on: Jan 16, 2019
 *      Author: 26sra
 */
#include "util.h"

int main(int argc, char ** argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: echo_client <host name> <port>\n");
		exit(EXIT_FAILURE);
	}
	// buffer for user input
	char userInput[BUFLEN];
	for (;;) {
		char buf[BUFLEN];
		memset(userInput, 0, BUFLEN);
		// read line
		if (fgets(userInput, BUFLEN, stdin) != NULL) {
			// constructs a new connection on every new line
			int sockFd = ClientConnectToServerTCP(argv[1], argv[2]);
			int userInputLength = strlen(userInput);
			int bytesSent = send(sockFd, userInput, userInputLength, 0);
			if (bytesSent!=userInputLength) {
				fprintf(stderr, "Error sending bytes (send()), "
					"printing errno below.\n%s", strerror(errno));
				exit(EXIT_FAILURE);
			}

			for (;;) { // just in case message is long and can't be read in 1 go
				ssize_t bytesRead = recv(sockFd, buf, BUFLEN, 0);
				if (bytesRead==-1) {
					continue;
				} else if (bytesRead>0) {
					// handle null terminator
					if (bytesRead < BUFLEN) buf[bytesRead] = 0;

#ifdef CLIENT_DEBUG
					printf("Client received %zd bytes from %s:\n",
							bytesRead, argv[1]);
#endif
					printf("%s", buf);
					break; // can safetly break now
				}
			}
			// tear down the connection and wait for
			// another new line to re-construct
			close(sockFd);
		}
	}
	return EXIT_SUCCESS;
}
