#ifndef PARSER_H_
#define PARSER_H_
#include "clients.h"

#define MAX_COMMAND_LENGTH 4
#define MAX_ARG_LENGTH 40

typedef struct pop3_command {
    char command[MAX_COMMAND_LENGTH + 1];
    char argument[MAX_ARG_LENGTH + 1];
}pop3_command;

struct pop3_command * fill_command(char * buffer);
int parse (struct pop3_command * command, struct client * client);

#endif