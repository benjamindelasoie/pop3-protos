#ifndef CLIENTS_H_
#define CLIENTS_H_

#include <unistd.h>
#include <stdbool.h>
#include "pop_commands.h"

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

typedef struct mail_file {
    char * file_name;
    int to_delete;
    struct mail_file * next;
}mail_file;

typedef struct client{
    unsigned int fd;

    client_state state;
    buffers buffers;
    char ** available_commands;
    int available_commands_count;
    command_function * available_commands_functions;
    
    char * username;
    char * mail_directory;
    struct mail_file * first_mail;

    bool user_auth;
}client;

void * handleClient (void * args);
ssize_t read_line(struct buffers * buffers);
void * set_mail_directory(void * args);
int fill_mail (struct client * client);
void free_client (struct client * client);

#endif