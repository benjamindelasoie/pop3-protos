#ifndef CLIENTS_H_
#define CLIENTS_H_

#include <unistd.h>

typedef enum client_state{
    GREETING,
    AUTHORIZATION,
    TRANSACTION,
    UPDATE
}client_state;


#define BUFSIZE 256

typedef struct buffers {
    char recieve[BUFSIZE+1];
    char parser[BUFSIZE+1];
    int recieve_index;
    int parser_index;
} buffers;


typedef struct client{
    unsigned int fd;

    client_state state;
    buffers buffers;
    char ** available_commands;
    int available_commands_count;
    
    int user;
}client;

void * handleClient (void * args);
ssize_t read_line(struct buffers * buffers);

#endif