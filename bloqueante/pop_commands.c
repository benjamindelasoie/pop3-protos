#include <string.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket

#include "parser.h"
#include "clients.h"
#include "pop_commands.h"

#define USER "user"
#define PASS "pass"

void user_command (struct pop3_command * command, struct client * client) {
    if (strcmp(command->argument, USER) == 0) {
        client->user_auth = true;
        send(client->fd, "+OK\r\n", 6, 0);
    } else {
        send(client->fd, "-ERR mailbox not found\r\n", 25, 0);
    }
    return;
}

void pass_command (struct pop3_command * command, struct client * client) {
    if (client->user_auth) {
        if (strcmp(command->argument, PASS) == 0) {
            client->state = TRANSACTION;
            client->available_commands = transaction_command;
            client->available_commands_count = transaction_command_count;
            client->available_commands_functions = transaction_command_function;
            send(client->fd, "+OK\r\n", 6, 0);
        } else {
            client->user_auth = false;
            send(client->fd, "-ERR\r\n", 7, 0);
        }
    } else {
        client->user_auth = false;
        send(client->fd, "-ERR\r\n", 7, 0);
    }
}

void quit_auth_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void quit_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
}

void stat_command (struct pop3_command * command, struct client * client) {
    send(client->fd, "+OK\r\n", 6, 0);
    return;
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
