#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "logger.h"
#include "tcpClientUtil.h"

#define BUFSIZE 512

void fill_buffer(char * buffer) {
	int i = 0;
	char c;
	while((c = getchar()) != '\n' && i<BUFSIZE-2) {
		buffer[i++] = c;
	}
	buffer[i++] = '\r';
	buffer[i++] = '\n';
}

void clear_buffer(char * buffer) {
	for(int i = 0; i<BUFSIZE; i++) {
		buffer[i] = 0;
	}
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		log(FATAL, "usage: %s <Server Name/Address> <Server Port/Name>", argv[0]);
	}

	char *server = argv[1];     // First arg: server name IP address 
	char * port = argv[2];		// Third arg server port

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port);
	if (sock < 0) {
		log(FATAL, "%s","socket() failed")
	}

	char buffer[BUFSIZE];

	char c;
	int flag = 1;
	while (recv(sock, buffer, BUFSIZE, 0) > 0) {
		printf("%s",buffer);
		fill_buffer(buffer);
		send(sock, buffer, strlen(buffer), 0);
		clear_buffer(buffer);
	}

	printf("closing...");

	close(sock);
	return 0;
}
