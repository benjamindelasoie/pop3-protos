#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "logger.h"
#include "parser.h"
#include "socket_util.h"
#include "clients.h" 
#include "pop_commands.h"


int main(int argc, char *argv[]) {
    unsigned port = 1080;
    
    if(argc == 2) {
        // utilizamos el default
        pthread_t tid;
        void * status;
        if(pthread_create(&tid, NULL, set_mail_directory, argv[1])) {
            log(FATAL, "%s", "mail directory not set properly");
        }
        pthread_join(tid, &status);
        if (status != NULL) {
            log(FATAL, "%s", "mail directory not set properly");
        }

    } else if(argc == 3){
        pthread_t tid;
        void * status;
        if(pthread_create(&tid, NULL, set_mail_directory, argv[1])) {
            log(FATAL, "%s", "mail directory not set properly");
        }
        pthread_join(tid, &status);
        if (status != NULL) {
            log(FATAL, "%s", "mail directory not set properly");
        }

        char *end     = 0;
        const long sl = strtol(argv[2], &end, 10);

        if (end == argv[2]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            log(FATAL, "port should be an integer: %s\n", argv[2]);
        }
        port = sl;
    }
	else {
		log(FATAL, "usage: %s <Mail Directory> <Server Port>", argv[0]);
	}
    const int server = setup_server_socket(port);

    while (1) {
        struct sockaddr_storage client_addr; // Client address
        // Set length of client address structure (in-out parameter)
        socklen_t client_addr_length = sizeof(client_addr);

        // Wait for a client to connect
        int client_socket = accept(server, (struct sockaddr *) &client_addr, &client_addr_length);
        if (client_socket < 0) {
            log(ERROR, "%s", "accept() failed");
            return -1;
        } else {
            // log(DEBUG, "Client %d connected", client_socket);
            pthread_t tid;
            if(pthread_create(&tid, NULL, handleClient, &client_socket)) {
                log(ERROR, "Could create client: %d", client_socket);
                return -1;
            }
            // fijarse como hacer esto en otro lado
            // pthread_join(tid, 0);
        }
    }

}