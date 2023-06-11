#ifndef PARSER_H_
#define PARSER_H_
#include "clients.h"

#define MAX_COMMAND_LENGTH 4
#define MAX_ARG_LENGTH 40

// AUTHENTICATION State 
static char * authorization_commands[] = {"USER", "PASS", "QUIT"};
static int authorization_command_count = 3;
// TRANSACTION State 
static char * transaction_command[] = {"QUIT", "STAT", "LIST", "RETR", "DELE", "NOOP", "RSET"};
static int transaction_command_count = 7;

typedef struct pop3_command {
    char command[MAX_COMMAND_LENGTH + 1];
    char argument[MAX_ARG_LENGTH + 1];
}pop3_command;

struct pop3_command * fill_command(char * buffer);
int parse (struct pop3_command * command, struct client * client);

#endif