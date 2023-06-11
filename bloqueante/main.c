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
#include "logger.h"
#include "parser.h"

#define BUFSIZE 256

typedef struct buffers {
    char recieve[BUFSIZE+1];
    char parser[BUFSIZE+1];
    int recieve_index;
    int parser_index;
} buffers;

static bool done = false;

static void
sigterm_handler(const int signal) {
    log(DEBUG, "signal %d, cleaning up and exiting\n",signal);
    done = true;
}

ssize_t read_line(struct buffers * buffers);

int main(int argc, char *argv[]) {
    unsigned port = 1080;
    
    if(argc == 1) {
        // utilizamos el default
    } else if(argc == 2){
        char *end     = 0;
        const long sl = strtol(argv[1], &end, 10);

        if (end == argv[1]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            log(FATAL, "port should be an integer: %s\n", argv[1]);
        }
        port = sl;
    }
	else {
		log(FATAL, "usage: %s <Server Port>", argv[0]);
	}

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        log(FATAL, "%s", "unable to create socket");
    }

    log(DEBUG, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        log(FATAL, "%s", "unable to bind socket");
    }

    if (listen(server, 20) < 0) {
        log(FATAL, "%s", "unable to listen");
    }

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

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
            log(DEBUG, "Client %d connected", client_socket);
            buffers client_buffers;
            memset(&client_buffers, 0, sizeof(client_buffers));
            ssize_t bytes_recieved = 0;
            
            while ((bytes_recieved = recv(client_socket, &(client_buffers.recieve[client_buffers.recieve_index]), BUFSIZE-client_buffers.recieve_index, 0)) > 0) {
                log(DEBUG, "%ld bytes recieved", bytes_recieved);
                log(DEBUG, "buffer: %s", client_buffers.recieve);
                int ready = read_line(&client_buffers);
                log(DEBUG, "parse_buffer: %s", client_buffers.parser);
                log(DEBUG, "ready: %d", ready);
                if (ready == 1) {
                    pop3_command * command = fill_command(client_buffers.parser);
                    client_buffers.parser_index = 0;
                    ssize_t num_bytes_sent;
                    if (command != NULL) {
                        int ok = parse(command, authentication_commands, authentication_command_count);
                        if (ok) {
                            num_bytes_sent = send(client_socket, "+OK!\r\n", 7, 0);
                        } else {
                            num_bytes_sent = send(client_socket, "-ERR!\r\n", 8, 0);
                        }
                    } else {
                        num_bytes_sent = send(client_socket, "-ERR!\r\n", 8, 0);
                    }
                    if (num_bytes_sent < 0) {
                        log(ERROR, "%s", "send() failed");
                    }
                }
            }
            if (bytes_recieved < 0) {
                log(ERROR, "%s", "recv() failed");
            } else {
                log(DEBUG, "closing client %d", client_socket);
                close(client_socket);
            }
        }
    }

}

//fills up paser emptying reciever
ssize_t read_line(struct buffers * buffers) {
    int crlf = 0;
    for ( ; buffers->recieve[buffers->recieve_index] != 0 && buffers->parser_index<BUFSIZE && buffers->recieve_index<BUFSIZE; 
        buffers->parser_index++, buffers->recieve_index++) {
        
        char c = buffers->recieve[buffers->recieve_index];
        buffers->parser[buffers->parser_index] = c;
        buffers->recieve[buffers->recieve_index] = 0;
        
        if( c == '\r' ) {
            crlf = 1;
        } else if (c == '\n') {
            if (crlf) {
                buffers->recieve_index++;
                return 1;
            } else {
                crlf = 0;
            }
        } else {
            crlf = 0;
        }
    }
    if (buffers->recieve_index >= BUFSIZE || buffers->recieve[buffers->recieve_index] == 0) {
        buffers->recieve_index = 0;
    }
    if (buffers->parser_index == BUFSIZE) {
        return 1;
    }
    return 0;
}