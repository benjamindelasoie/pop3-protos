#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "logger.h"
#include "tcpServerUtil.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
		log(FATAL, "usage: %s <Server Port>", argv[0]);
	}

	char * servPort = argv[1];

	int servSock = runServer();
	if (servSock < 0 ) {
		log(ERROR, "couldn't create server socket")
	}

	while (1) { // Run forever
		// Wait for a client to connect
		int clntSock = acceptConnection(servSock);
		if (clntSock < 0) {
			log(ERROR, "accept() failed");
    }
		else {
			handleTCPEchoClient(clntSock);
		}
	}

  return 0;

}