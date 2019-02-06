/*
 * util.h
 *
 *  Created on: Jan 16, 2019
 *      Author: sramirez266
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "signal.h"

#define SERVER_DEBUG
#define CLIENT_DEBUG
#define BUFLEN 1024
#define CLOSED_SOCKET -1

typedef struct ServerTCPMessage {
	int tcpFd;
	unsigned char * buf;
	ssize_t bytesOut;
	char * clientName;
	char * clientService;
}ServerTCPMessage;

typedef void (*OnReceiveDelegate) (ServerTCPMessage * serverMsg);
typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr sockaddr;

int ServerInitTCP(char * port, int backlog);
int ClientConnectToServerTCP(char * host, char * port);
void ServerSpinTCP(int tcpFd, OnReceiveDelegate callback);
#endif /* UTIL_H_ */
