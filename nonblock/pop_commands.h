#ifndef _POP_COMMANDS_H_
#define _POP_COMMANDS_H_

#include "parser.h"

static char * mail_directory;

struct pop3_command;
struct client;

//authorization commands
void user_command (struct pop3_command * command, struct client * client);
void pass_command (struct pop3_command * command, struct client * client);
void quit_auth_command (struct pop3_command * command, struct client * client);

//transaction commands
void quit_command (struct pop3_command * command, struct client * client);
void stat_command (struct pop3_command * command, struct client * client);
void list_command (struct pop3_command * command, struct client * client);
void retr_command (struct pop3_command * command, struct client * client);
void dele_command (struct pop3_command * command, struct client * client);
void noop_command (struct pop3_command * command, struct client * client);
void rset_command (struct pop3_command * command, struct client * client);

// AUTHENTICATION State 
static char * authorization_command[] = {"QUIT", "USER", "PASS"};
static int authorization_command_count = 3;
// TRANSACTION State 
static char * transaction_command[] = {"QUIT", "STAT", "LIST", "RETR", "DELE", "NOOP", "RSET"};
static int transaction_command_count = 7;

typedef void (*command_function) (struct pop3_command *, struct client *);

static command_function authorization_command_function[] = 
    {(command_function)&quit_auth_command,
    (command_function)&user_command, 
    (command_function)&pass_command
    };
static command_function transaction_command_function[] = 
    {(command_function) &quit_command, 
    (command_function) &stat_command, 
    (command_function) &list_command, 
    (command_function) &retr_command, 
    (command_function) &dele_command, 
    (command_function) &noop_command, 
    (command_function) &rset_command};

#endif