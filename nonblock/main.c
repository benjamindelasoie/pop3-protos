#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>  // socket
#include <signal.h>

#include "logger.h"
#include "io_handler.h"
#include "client.h"
#include "socket_util.h"
#include "client_util.h"

static bool done = false;
struct client * clients[MAX_CLIENTS];

static void sigterm_handler(const int signal) {
    log(DEBUG, "signal %d, cleaning up and exiting\n",signal);
    done = true;
    for(int i=0; i<MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            close(i);
            remove_logged_user(clients[i]);
            free_client(clients[i]);
        }
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    unsigned port = 1080;

    if(argc == 2) {
        // utilizamos el default
        mail_directory = argv[1];
    } else if(argc == 3) {
        mail_directory = argv[1];
        char *end     = 0;
        const long sl = strtol(argv[2], &end, 10);

        if (end == argv[2]|| '\0' != *end 
           || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
           || sl < 0 || sl > USHRT_MAX) {
            log(FATAL, "port should be an integer: %s\n", argv[2]);
        }
        port = sl;
    } else {
        log(FATAL, "usage: %s <Mail Directory> <Server Port>", argv[0]);
    }

    close(0);
    const int server = setup_server_socket(port);
    if (set_socket_nonblock (server) < 0) {
        log(FATAL, "%s", "Could not set server to nonblock");
    }

    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    struct select_info select_info;
    select_info.max_fd = 0;
    FD_ZERO(&select_info.readfds);
    FD_ZERO(&select_info.writefds);

    int activity;

    while(true) {
        //set interests
        FD_SET(server, &select_info.readfds);
        if (select_info.max_fd < server) {
            select_info.max_fd = server;
        }
        for (int i=0; i<MAX_CLIENTS; i++) {
            if (clients[i] != NULL) {
                if (clients[i]->interest == READ) {
                    FD_SET(i, &select_info.readfds);
                    if (i > select_info.max_fd) {
                    select_info.max_fd = i;
                    }
                } else if (clients[i]->interest == WRITE){
                    FD_SET(i, &select_info.writefds);
                    if (i > select_info.max_fd) {
                    select_info.max_fd = i;
                    }
                }
            }
        }

        activity = select(select_info.max_fd + 1, &select_info.readfds, &select_info.writefds, NULL, NULL);

        if (activity < 0) {
            for(int i=0; i<MAX_CLIENTS; i++) {
                if (clients[i] != NULL) {
                    close(i);
                    free_client(clients[i]);
                    clients[i] = NULL;
                }
            }
            log(FATAL, "Select error, aborting %d", errno);
        }

        if (FD_ISSET(server, &select_info.readfds)) {
            struct sockaddr_storage client_addr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t client_addr_length = sizeof(client_addr);

            // Wait for a client to connect
            int client_socket = accept(server, (struct sockaddr *) &client_addr, &client_addr_length);
            if (INVALID_FD(client_socket)) {
                // close(client_socket);
                log(ERROR, "%s", "accept() failed");
                // return -1;
            } else { 
                clients[client_socket] = calloc(1, sizeof(struct client));
                if (clients[client_socket] == NULL) {
                    close(client_socket);
                    log(ERROR, "%s","Malloc failed");
                } else {
                    suscribe_ok(clients[client_socket]);
                    clients[client_socket]->mail_directory = mail_directory;
                    clients[client_socket]->fd = client_socket;
                    clients[client_socket]->state = AUTHORIZATION;
                    clients[client_socket]->available_commands = authorization_command;
                    clients[client_socket]->available_commands_count = authorization_command_count;
                    clients[client_socket]->available_commands_functions = authorization_command_function;
                    set_socket_nonblock (client_socket);
                }
            }
        }

        for (int i = 0; i< MAX_CLIENTS; i++) {
            if (clients[i] != NULL) {
                if (FD_ISSET(i, &select_info.readfds)) {
                    FD_CLR(i, &select_info.readfds);
                    if(handle_read(clients[i]) <= 0) {
                        remove_logged_user(clients[i]);
                        free_client(clients[i]);
                        close(i);
                        clients[i] = NULL;
                    }
                }
            }
        }
        for (int i = 0; i< MAX_CLIENTS; i++) {
            if (clients[i] != NULL) {
                if (FD_ISSET(i, &select_info.writefds)) {
                    FD_CLR(i, &select_info.writefds);
                    if (handle_write(clients[i]) <= 0) {
                        remove_logged_user(clients[i]);
                        free_client(clients[i]);
                        close(i);
                        clients[i] = NULL;
                    }
                }
            }
        }

    }
}