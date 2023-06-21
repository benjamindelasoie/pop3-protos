#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "parser.h"
#include "client.h"
#include "pop_commands.h"
#include "logger.h"
#include "client_util.h"
#include "io_handler.h"

#define USER "user"
#define PASS "pass"

void user_command (struct pop3_command * command, struct client * client) {
    
    FILE * file = fopen("users.txt", "r");
    char buffer[BUFSIZE + 1] = {0};
    int arg_length = strlen(command->argument);

    while (fgets(buffer, BUFSIZE, file) != NULL) {
        int flag = 1;
        int i;
        for(i=0; buffer[i]!=0 && command->argument[i]!= 0 && flag && i<arg_length &&i<BUFSIZE; i++) {
            if (command->argument[i] != buffer[i]) {
                flag = 0;
            }
        }

        if (flag) {
            if (i == arg_length) {
                client->user_auth = true;
                client->username = calloc(1,strlen(command->argument) + 1);
                strcpy(client->username, command->argument);
                // send(client->fd, "+OK\r\n", 6, 0);
                suscribe_ok(client);
                return;
            } 
        }
    }

    suscribe_err(client);
    // send(client->fd, "-ERR mailbox not found\r\n", 25, 0);
    return;
}

void pass_command (struct pop3_command * command, struct client * client) {
    
    if (client->user_auth) {

        FILE * file = fopen("users.txt", "r");
        char buffer[BUFSIZE + 1] = {0};
        int arg_length = strlen(command->argument);
        
        while (fgets(buffer, BUFSIZE, file) != NULL) {
            int flag = 1;
            int i, j=0;

            while (buffer[j] != 0 && buffer[j++] != ' ');

            for(i=0; buffer[j]!=0 && command->argument[i]!= 0 && flag && i<arg_length && j<BUFSIZE; i++, j++) {
                if (command->argument[i] != buffer[j]) {
                    flag = 0;
                }
            }

            if (flag) {
                if (i == arg_length) {
                    if (fill_mail(client) < 0) {
                        //TODO: CHeck errors
                        return;
                    }
                    client->state = TRANSACTION;
                    client->available_commands = transaction_command;
                    client->available_commands_count = transaction_command_count;
                    client->available_commands_functions = transaction_command_function;
                    // send(client->fd, "+OK\r\n", 6, 0);
                    suscribe_ok(client);
                    return;
                } 
            }
        }
        client->user_auth = false;
        free(client->username);
        suscribe_err(client);
        // send(client->fd, "-ERR incorrect credentials, try again.\r\n", 41, 0);
    } else {
        // client->user_auth = false;
        // free(client->username);
        // send(client->fd, "-ERR no user provided\r\n", 24, 0);
        suscribe_err(client);
    }
}

void quit_auth_command (struct pop3_command * command, struct client * client) {
    // send(client->fd, "+OK Logging out\r\n", 18, 0);
    suscribe_ok(client);
    client->state = CLOSING;
    return;
}

void quit_command (struct pop3_command * command, struct client * client) {
    struct mail_file * current = client->first_mail;

    while(current != NULL) {
        if (current->to_delete == 1) {
            remove(current->file_name);
        }
        current = current->next;
    }

    // send(client->fd, "+OK Logging out\r\n", 18, 0);
    suscribe_ok(client);
    client->state = CLOSING;
    return;
}

void stat_command (struct pop3_command * command, struct client * client) {
    int file_count = 0;
    off_t dir_size = 0;
    struct stat st;
    // char buffer[BUFSIZE+1] = {0};
    
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

    sprintf(client->buffers.write, "+OK %d %lo\r\n", file_count, dir_size);
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    // sprintf(buffer, "+OK %d %lo\r\n", file_count, dir_size);
    // send(client->fd, buffer, strlen(buffer), 0);
}

void list_command (struct pop3_command * command, struct client * client) {
    struct stat st;
    int not_deleted = 0;
    off_t dir_size = 0;
    struct mail_file * current = client->first_mail;
    int argument_id;

    if(command->argument[0] != '\0'){
        argument_id = atoi(command->argument);
    }else{
        while(current != NULL){
            if (current->to_delete == 0) {
                if (stat(current->file_name, &st) >= 0) {
                    dir_size+=st.st_size;
                }else if (errno == ENOENT) {
                    log(ERROR, "%s", "Could not access file");
                } else {
                    log(ERROR, "%d", errno);
            }
            not_deleted++;
            }
            current = current->next;
        }

        sprintf(client->buffers.write, "+OK %d messages (%lo octets)\r\n", not_deleted, dir_size);
        client->buffers.write_size = strlen(client->buffers.write);
        suscribe_write(client);
    }


    current = client->first_mail;

    if(command->argument[0] != '\0'){
        while (current != NULL) {
            if (current->to_delete == 0) {
                    if (argument_id == current->id)
                    {
                        if (stat(current->file_name, &st) >= 0){
                            sprintf(client->buffers.write, "+OK %d %lo\r\n", current->id, st.st_size);
                            client->buffers.write_size = strlen(client->buffers.write);
                            suscribe_write(client);
                        }else if (errno == ENOENT) {
                            log(ERROR, "%s", "Could not access file");
                        } else {
                            log(ERROR, "%d", errno);
                        }
                        return;
                    }
                    not_deleted++;
                }
        }

        sprintf(client->buffers.write, "-ERR no such message, only %d messages in mail\r\n", not_deleted);\
        client->buffers.write_size = strlen(client->buffers.write);
        suscribe_write(client);
    }else{
        client->current_mail = current;
        fill_list_command(client);
    }
    
    return;
}

void fill_list_command(struct client * client)
{
    struct stat st;
    struct mail_file * current = client->current_mail;

    while (current != NULL) {
        if (current->to_delete == 0) {
            
            if(client->buffers.write_size == BUFSIZE){
                client->state = WRITING_LIST;
                client->current_mail = current;
                return;
            }else if (stat(current->file_name, &st) >= 0) {
                sprintf(client->buffers.write, "+OK %d %lo\r\n", current->id, st.st_size);
                client->buffers.write_size = strlen(client->buffers.write);
                suscribe_write(client);
            }else if (errno == ENOENT) {
                log(ERROR, "%s", "Could not access file");
            } else {
                log(ERROR, "%d", errno);
            }
        }
        current = current->next;
    }

    return;
}

void retr_command (struct pop3_command * command, struct client * client) {
    char *end = 0;
    const long sl = strtol(command->argument, &end, 10);
    struct mail_file * current = client->first_mail;
    char buffer[BUFSIZE+1] = {0};
    FILE * file;
    int bytes_read = 0;
    int i = 1;

    while(current != NULL) {
        if (i == sl && current->to_delete == 0) {
            if ((file = fopen(current->file_name, "r")) == NULL) {
                send(client->fd, "-ERR Could not open mail\r\n", 27, 0);
            } else {
                send(client->fd, "+OK\r\n", 6, 0);
                while ( fgets(buffer, BUFSIZE, file) != NULL) {
                    send(client->fd, buffer, strlen(buffer), 0);
                }
                send(client->fd, "\r\n.\r\n", 6, 0);
            }
            fclose(file);
            return;
        }
        current = current->next;
        i++;
    }

    send(client->fd, "-ERR No such mail\r\n", 20, 0);

    return;
}

void dele_command (struct pop3_command * command, struct client * client) {
    char *end = 0;
    const long sl = strtol(command->argument, &end, 10);
    struct mail_file * current = client->first_mail;
    int i = 1;

    while(current != NULL) {
        if (i == sl && current->to_delete == 0) {
            current->to_delete = 1;
            suscribe_ok(client);
            // send(client->fd, "+OK message deleted\r\n",22,0);
            return;
        }
        current = current->next;
        i++;
    }

    suscribe_err(client);
    // send(client->fd, "-ERR No such mail\r\n", 20, 0);
    return;
}

void noop_command (struct pop3_command * command, struct client * client) {
    // send(client->fd, "+OK\r\n", 6, 0);
    suscribe_ok(client);
    return;
}

void rset_command (struct pop3_command * command, struct client * client) {
    struct mail_file * current = client->first_mail;

    while(current != NULL) {
        current->to_delete = 0;
        current = current->next;
    }

    // send(client->fd, "+OK\r\n", 6, 0);
    suscribe_ok(client);
    return;
}
