#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <unistd.h>
#include <stdbool.h>
#include "pop_commands.h"



typedef enum client_state{
    GREETING,
    AUTHORIZATION,
    TRANSACTION,
    UPDATE,
    CLOSING,
    WRITING_LIST,
    WRITING_MAIL
}client_state;

typedef enum interest {
    NOOP,
    READ,
    WRITE,
    READ_FILE
} interest;

#define BUFSIZE 8192

typedef struct buffers {
    char recieve[BUFSIZE+1];
    char parser[BUFSIZE+1];
    char write[BUFSIZE+1];
    int bytes_recieved;
    int recieve_index;
    int parser_index;
    int write_index;
    int write_size;
} buffers;

typedef struct mail_file {
    char * file_name;
    int to_delete;
    int id;
    struct mail_file * next;
}mail_file;

typedef struct client{
    unsigned int fd;

    client_state state;
    interest interest;
    buffers buffers;
    char ** available_commands;
    int available_commands_count;
    command_function * available_commands_functions;
    
    char * username;
    char * mail_directory;
    struct mail_file * first_mail;
    struct mail_file * current_mail;
    int mail_fd;

    // metricas 
    struct metrics * metricas;

    bool user_auth;
}client;

#endif