//============================================================================
// Name        : IO_TcpServer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

//#include <iostream>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
//using namespace std;

#define MAXDATASIZE 100
#define  PORT 1234
#define BACKLOG 5

typedef struct CLIENT {
	int fd;
	char* name;
	struct sockaddr_in addr;
	char* data;
};

void process_cli(CLIENT *client, char * recvbuf, int len);
void savedata(char* recvbuf, int len, char * data);

int main() {
	int i, maxi, maxfd, sockfd;
	int nready;
	ssize_t n;
	fd_set rset, allset;
	int listenfd, connectfd;
	struct sockaddr_in server;

	CLIENT client[FD_SETSIZE];
	char recvbuf[MAXDATASIZE];
	int sin_size;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket() error.");
		exit(1);
	}

	int opt = SO_REUSEADDR;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))
			== -1) {
		perror("setsockopt() error ");
		exit(1);
	}

	bzero(&server, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
//		cout << server.sin_addr.s_addr << endl;
		perror("bind() error");
		exit(1);
	}

//	cout << "bind finish" << endl;
	if (listen(listenfd, BACKLOG) == -1) {
		perror("listen() error");
		exit(1);
	}
//	cout << "listen finish" << endl;

	sin_size = sizeof(struct sockaddr_in);

	maxfd = listenfd;
	maxi = -1;
	for (i = 0; i < FD_SETSIZE; ++i) {
		client[i].fd = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	while (1) {
		struct sockaddr_in addr;
		rset = allset;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {
			if ((connectfd = accept(listenfd, (struct sockaddr *) &addr, &sin_size))
					== -1) {
				perror("accept() error");
				exit(1);
			}
		}
	}

	return 0;
}
