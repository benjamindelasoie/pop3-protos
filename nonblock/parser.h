#ifndef _PARSER_H_
#define _PARSER_H_

#define MAX_COMMAND_LENGTH 4
#define MAX_ARG_LENGTH 40

struct client;
struct buffers;

typedef struct pop3_command {
    char command[MAX_COMMAND_LENGTH + 1];
    char argument[MAX_ARG_LENGTH + 1];
}pop3_command;

struct pop3_command * fill_command(char * buffer);
int parse (struct pop3_command * command, struct client * client);
int read_line(struct buffers * buffers);

#endif