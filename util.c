/*
 * util.c
 *
 *  Created on: Jan 16, 2019
 *      Author: sramirez266
 */


#include "util.h"

static volatile sig_atomic_t SIG_FLAG;
static void sigint_handler(int sig) {
	(void)sig;
	SIG_FLAG = 1;
}

// simple checks on validity of port string passed
static long int CheckValidPort(char * str) {
	char * endptr;
	long int val;
	val = strtol(str, &endptr, 10);
	if ((errno == ERANGE && (val == INT_MAX || val == INT_MIN))
		   || (errno != 0 && val == 0)) {
	   fprintf(stderr, "invalid port number: '%s'\n", str);
	   exit(EXIT_FAILURE);
	} else if (endptr == str) {
		fprintf(stderr, "No digits were found\n");
		exit(EXIT_FAILURE);
	} else if (val < 1024) {
		fprintf(stderr, "port numbers below 1024 reserved: '%s'\n", str);
		exit(EXIT_FAILURE);
	}
	return val;
}

typedef struct thread_context_t{
	OnReceiveDelegate callback;
	ServerTCPMessage * msg;
}thread_context_t;

static void *
ThreadedCallback(void * arg) {
	thread_context_t * tc = (thread_context_t *) arg;
	tc->callback(tc->msg);
}
// takes the listening TCP socket returned from ServerInitTCP()
// and a callback function to perform a specific task
// on any successful data receive event.
// server spin in a while loop. on each iteration,
// 1. accept an incoming connection,
// 2. get information on the client connection.
// 3. receive data from client
// 4. run a specific call back defined in the server program
void ServerSpinTCP(int tcpFd, OnReceiveDelegate callback) {
	int acceptFd = CLOSED_SOCKET;
	ssize_t bytesRead = 0;
	unsigned char buf[BUFLEN] = {0};
	char clientName[NI_MAXHOST];
	char clientService[NI_MAXSERV];

	// register signal handler
	// to properly close socket on Ctr-c
	SIG_FLAG = 0;
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGINT, &sa, NULL)==-1) {
		perror("sigaction");
		exit(1);
	}

	while (!SIG_FLAG) {
		sockaddr clientAddr;
		socklen_t addrSize = sizeof(clientAddr);
		// now accept an incoming connection
		if ((acceptFd = accept(tcpFd, &clientAddr, &addrSize)) == -1) {
			fprintf(stderr, "accept() error occured\n");
			goto clean;
		}
		int rv = getnameinfo(&clientAddr, addrSize,
				clientName, NI_MAXHOST, clientService,
				NI_MAXSERV, NI_NUMERICSERV);

		if ((bytesRead = recv(acceptFd, buf, BUFLEN, 0))==-1) {
			close(acceptFd);
			continue; // ignore failed request
		}

		if (rv==0 && bytesRead>0) {
			// handle null terminator
			if (bytesRead < BUFLEN) buf[bytesRead] = 0;

#ifdef SERVER_DEBUG
			printf("ServerSpinTCP::Server received %zd bytes from %s:\n%s\n",
					bytesRead, clientName, buf);
#endif
			if (callback != NULL) {
				pthread_t worker;
				ServerTCPMessage serverMsg;
				thread_context_t context;

				serverMsg.buf = buf;
				serverMsg.bytesOut = bytesRead;
				serverMsg.clientName = clientName;
				serverMsg.clientService = clientService;
				serverMsg.tcpFd = acceptFd;
				context.msg = serverMsg;
				context.callback = callback;
				pthread_create(&worker, NULL, ThreadedCallback, &context);
//				callback(&serverMsg);
			}
		} else if (rv!=0) {
			fprintf(stderr, "getnameinfo() error: %s\n", gai_strerror(rv));
			goto clean;
		}
		close(acceptFd);
		acceptFd = CLOSED_SOCKET;
	}
	// clean up on SIGINT
	clean:
		if (tcpFd != CLOSED_SOCKET) close(tcpFd);
		if (acceptFd != CLOSED_SOCKET) close(acceptFd);

#ifdef SERVER_DEBUG
		char msg[] = "\nSocket clean up complete\n";
		// async safe, +1 to suppress compiler warnings
		(void)(write(0, msg, sizeof(msg))+1);
#endif
		exit(EXIT_FAILURE);
}

// Constructs a TCP connection for a client
// to a server and returns the
// socket file descriptor
int ClientConnectToServerTCP(char * host, char * port) {
	int status, sockFd;
	addrinfo hints;
	addrinfo * thisInfo;  // will point to the results
	addrinfo * cur;
	CheckValidPort(port);
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	// get ready to connect
	if ((status = getaddrinfo(host, port, &hints, &thisInfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	// let the kernel choose the local port with connect() instead of bind()
	for (cur=thisInfo; cur!=NULL; cur=cur->ai_next) {
		sockFd = socket(cur->ai_family,	cur->ai_socktype, cur->ai_protocol);
		if (sockFd == -1) {
			continue; // unsuccessful address structure
		} else if (connect(sockFd, cur->ai_addr, cur->ai_addrlen)==0) {
			break; // successful connection
		}
		close(sockFd);
	}

	if (cur==NULL) {
		fprintf(stderr, "Could not connect socket\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(thisInfo);
	return sockFd;
}

// Sets up the listening socket on the server
// for a TCP connection
int ServerInitTCP(char * port, int backlog) {
	int status, listenFd;
	addrinfo hints;
	addrinfo * thisInfo;
	CheckValidPort(port);
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	if ((status = getaddrinfo(NULL, port, &hints, &thisInfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	// make a socket, bind it, and listen on it:
	listenFd = socket(thisInfo->ai_family, thisInfo->ai_socktype, thisInfo->ai_protocol);
	bind(listenFd, thisInfo->ai_addr, thisInfo->ai_addrlen);
	listen(listenFd, backlog);
	freeaddrinfo(thisInfo); // free the linked-list
	return listenFd;
}

