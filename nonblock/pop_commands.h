#ifndef _POP_COMMANDS_H_
#define _POP_COMMANDS_H_

#include "parser.h"
#include "errors.h"

static char * mail_directory;

struct pop3_command;
struct client;

//authorization commands
return_status user_command (struct pop3_command * command, struct client * client);
return_status pass_command (struct pop3_command * command, struct client * client);
return_status quit_auth_command (struct pop3_command * command, struct client * client);

//transaction commands
return_status quit_command (struct pop3_command * command, struct client * client);
return_status stat_command (struct pop3_command * command, struct client * client);
return_status list_command (struct pop3_command * command, struct client * client);
return_status retr_command (struct pop3_command * command, struct client * client);
return_status dele_command (struct pop3_command * command, struct client * client);
return_status noop_command (struct pop3_command * command, struct client * client);
return_status rset_command (struct pop3_command * command, struct client * client);

// monitor commands
return_status monitor_login_command (struct pop3_command * command, struct client * client); // ESTE VA ESTADO DE AUTHORIZATION
return_status historical_command (struct pop3_command * command, struct client * client);
return_status concurrent_command (struct pop3_command * command, struct client * client);
return_status bytes_sent_command (struct pop3_command * command, struct client * client);


void fill_list_command(struct client * client);

// AUTHENTICATION State 
static char * authorization_command[] = {"QUIT", "USER", "PASS", "MONI"};
static int authorization_command_count = 4;
// TRANSACTION State 
static char * transaction_command[] = {"QUIT", "STAT", "LIST", "RETR", "DELE", "NOOP", "RSET"};
static int transaction_command_count = 7;

// monitor commands
static char * monitor_commands[] = { "QUIT", "CONH", "CONC", "BYTS", };
static int monitor_commands_count = 4;

static char * monitor_auth_commands[] = { "MONI",  "QUIT" };
static int monitor_auth_commands_count = 2;

typedef return_status (*command_function) (struct pop3_command *, struct client *);

static command_function authorization_command_function[] = 
{
    (command_function) &quit_auth_command,
    (command_function) &user_command, 
    (command_function) &pass_command
};

static command_function monitor_auth_command_function[] =
{
    (command_function) &monitor_login_command,
    (command_function) &quit_auth_command
};

static command_function transaction_command_function[] = 
{
    (command_function) &quit_command, 
    (command_function) &stat_command, 
    (command_function) &list_command, 
    (command_function) &retr_command, 
    (command_function) &dele_command, 
    (command_function) &noop_command, 
    (command_function) &rset_command
};

static command_function monitor_command_function[] = 
{
    (command_function) &quit_auth_command,
    (command_function) &historical_command,
    (command_function) &concurrent_command,
    (command_function) &bytes_sent_command,
};

#endif