#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "parser.h"

struct pop3_command * fill_command(char * buffer) {
    pop3_command * command = calloc(1, sizeof(pop3_command));
    if (command == NULL) {
        return NULL;
    }
    
    int buff_index = 0;
    int i=0;
    for(; buffer[buff_index] != ' ' && buffer[buff_index] != '\r' && i < MAX_COMMAND_LENGTH; i++, buff_index++) {
        command->command[i] = buffer[buff_index];
        buffer[buff_index] = 0;
    }
    command->command[i] = 0;

    if (buffer[buff_index] != '\r' && buffer[buff_index] != ' ') {
        //vacio buffer
        while(buffer[buff_index] != 0) {
            buffer[buff_index++] = 0;
        }
        //error command not valid
        free (command);
        return NULL;
    }

    if (buffer[buff_index] == ' ') {
        //consume space
        buff_index++;
    } else {
        return command;
    }

    int j = 0;
    for(; buffer[buff_index] != ' ' && buffer[buff_index] != '\r' && j < MAX_ARG_LENGTH; j++, buff_index++) {
        command->argument[j] = buffer[buff_index];
        buffer[buff_index] = 0;
    }
    command->argument[j] = 0;

    if (buffer[buff_index] != '\r' || buffer[buff_index+1] != '\n') {
        //vacio buffer
        while(buffer[buff_index] != 0) {
            buffer[buff_index++] = 0;
        }
        //error argument not valid
        free (command);
        return NULL;
    }

    return command;
}

int parse (struct pop3_command * command, char * valid_commands[], int command_count) {

    for(int i = 0; i < command_count; i++) {
        if (strcasecmp(command->command, valid_commands[i]) == 0) {
            free(command);
            return 1;
        }
    }
    free(command);
    return 0;
}