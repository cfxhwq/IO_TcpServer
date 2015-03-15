//============================================================================
// Name        : IO_TcpServer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdlib.h>
using namespace std;

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
	socklen_t sin_size;

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
		cout << server.sin_addr.s_addr << endl;
		perror("bind() error");
		exit(1);
	}

	cout << "bind finish" << endl;
	if (listen(listenfd, BACKLOG) == -1) {
		perror("listen() error");
		exit(1);
	}
	cout << "listen finish listenfd=" << listenfd << endl;

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
		if (&rset != 0  )
			if (FD_ISSET(listenfd, &rset)) {
				cout << "listenfd exit in reset" << endl;
			}
		rset = allset;
		cout << "copy allset to rset finish" << endl;
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
		cout << "select finish nready=" << nready << endl;

		if (FD_ISSET(listenfd, &rset)) {
			if ((connectfd = accept(listenfd, (struct sockaddr *) &addr,
					&sin_size)) == -1) {
				perror("accept() error");
				continue;
			} else {
				cout << "connectfd=" << connectfd << endl;
			}
			for (i = 0; i < FD_SETSIZE; ++i) {
				cout << "finding" << endl;
				if (client[i].fd < 0) {
					cout << "found in client " << i << endl;
					client[i].fd = connectfd;
					client[i].name = new char[MAXDATASIZE];
					client[i].data = new char[MAXDATASIZE];
					client[i].addr = addr;
					client[i].name[0] = '\0';
					client[i].data[0] = '\0';
					printf("connect from %s\n",
							inet_ntoa(client[i].addr.sin_addr));
					break;
				}
			}
			cout << "put into client finish" << endl;
			if (i == FD_SETSIZE)
				printf("too many client!\n");
			FD_SET(connectfd, &allset);
			if (connectfd > maxfd)
				maxfd = connectfd;
			if (i > maxi)
				maxi = i;
			if (--nready <= 0) {
				cout << "not one ready" << endl;
				continue;
			}
		} else {
			cout << "listenfd is not in rset" << endl;
		}
		cout << "begin recv or send" << endl;
		for (i = 0; i <= maxi; ++i) {
			if ((sockfd = client[i].fd) < 0)
				continue;
			else
				cout << "sockfd=" << sockfd << endl;
			if (FD_ISSET(sockfd, &rset)) {
				if ((n = recv(sockfd, recvbuf, MAXDATASIZE, 0)) == 0) {
					close(sockfd);
					printf("client:%s closed,data:%s\n", client[i].name,
							client[i].data);
					FD_CLR(sockfd, &allset);
					client[i].fd = -1;
					free(client[i].name);
					free(client[i].data);
				} else {
					process_cli(&client[i], recvbuf, n);
				}
				if (--nready <= 0) {
					cout << "nready <= 0" << endl;
					break;
				}
			} else {
				cout << "sock not in rset" << endl;
			}
		}
		cout << "all finish" << endl;
	}
	close(listenfd);
	return 0;
}

void process_cli(CLIENT *client, char * recvbuf, int len) {
	char sendbuf[MAXDATASIZE];
	recvbuf[len - 1] = '\0';
	if (strlen(client->name) == 0) {
		memcpy(client->name, recvbuf, len);
		printf("client name:%s\n", client->name);
		return;
	}

	printf("get client %s messsage:%s\n", client->name, recvbuf);

	savedata(recvbuf, len, client->data);

	for (int i1 = 0; i1 < len - 1; ++i1) {
		sendbuf[i1] = recvbuf[len - i1 - 2];
	}
	sendbuf[len - 1] = '\0';
	send(client->fd, sendbuf, strlen(sendbuf), 0);
}

void savedata(char* recvbuf, int len, char* data) {
	int start = strlen(data);
	for (int i = 0; i < len; ++i) {
		data[start + i] = recvbuf[i];
	}
}

