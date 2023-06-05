#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/socket.h>


// Create, bind, and listen a new TCP server socket
int runServer();

// Accept a new TCP connection on a server socket
int acceptConnection(int servSocket);

// Handle new TCP client
void handleConnection(int clntSocket);

#endif 
