#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "parser.h"
#include "clients.h"
#include "pop_commands.h"
#include "logger.h"

#define USER "user"
#define PASS "pass"

void user_command (struct pop3_command * command, struct client * client) {
    if (strcmp(command->argument, USER) == 0) {
        client->user_auth = true;
        client->username = calloc(1,strlen(command->argument) + 1);
        strcpy(client->username, command->argument);
        send(client->fd, "+OK\r\n", 6, 0);
    } else {
        send(client->fd, "-ERR mailbox not found\r\n", 25, 0);
    }
    return;
}

void pass_command (struct pop3_command * command, struct client * client) {
    if (client->user_auth) {
        if (strcmp(command->argument, PASS) == 0) {
            if (fill_mail(client) < 0) {
                //TODO: CHeck errors
                return;
            }
            client->state = TRANSACTION;
            client->available_commands = transaction_command;
            client->available_commands_count = transaction_command_count;
            client->available_commands_functions = transaction_command_function;
            send(client->fd, "+OK\r\n", 6, 0);
        } else {
            client->user_auth = false;
            free(client->username);
            send(client->fd, "-ERR\r\n", 7, 0);
        }
    } else {
        client->user_auth = false;
        free(client->username);
        send(client->fd, "-ERR\r\n", 7, 0);
    }
}

void quit_auth_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK Logging out\r\n", 18, 0);
    return;
}

void quit_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK Logging out\r\n", 18, 0);
    return;
}

void stat_command (struct pop3_command * command, struct client * client) {
    int file_count = 0;
    off_t dir_size = 0;
    struct stat st;
    char buffer[BUFSIZE+1] = {0};
    
    struct mail_file * current = client->first_mail;

    while (current != NULL) {
        if (current->to_delete == 0) {
            if (stat(current->file_name, &st) >= 0) {
                file_count++;
                dir_size += st.st_size;
            } else if (errno == ENOENT) {
                log(ERROR, "%s", "Could not access file");
            } else {
                log(ERROR, "%d", errno);
            }
        }
        current = current->next;
    }

    sprintf(buffer, "+OK %d %lo\r\n", file_count, dir_size);
    send(client->fd, buffer, strlen(buffer), 0);
}

void list_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void retr_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void dele_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void noop_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void rset_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}
