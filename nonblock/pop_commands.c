#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

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
#define MONITOR_LOGIN_TOKEN "MAGIA"

typedef struct logged_in{
    char * username;
    struct logged_in * next;
}logged_in;

logged_in * first_logged_in_user = NULL;
logged_in * current;

return_status user_command (struct pop3_command * command, struct client * client) {
    current = first_logged_in_user;
    while (current != NULL)
    {
        if(strcmp(current->username, command->argument) == 0){
            return suscribe_err(client);
        }
        current = current->next;
    }

    FILE * file = fopen("users.txt", "r");
    if (file == NULL) {
        return FILE_ERR;
    }
    char buffer[BUFSIZE + 1] = {0};
    int arg_length = strlen(command->argument);

    if (command->argument[0] == 0) {
        return suscribe_err(client);
    }

    while (fgets(buffer, BUFSIZE, file) != NULL) {
        int flag = 1;
        int i;
        for(i=0; buffer[i]!=0 && command->argument[i]!= 0 && flag && i<arg_length &&i<BUFSIZE; i++) {
            if (command->argument[i] != buffer[i]) {
                flag = 0;
            }
        }

        if (flag && buffer[i] == ',' && i == arg_length) {
            client->user_auth = true;
            client->username = calloc(1,strlen(command->argument) + 1);
            if (client->username == NULL) {
                return MEMORY_ALOC;
            }
            strcpy(client->username, command->argument);
            return suscribe_ok(client);
        }
    }

    return suscribe_err(client);
}

return_status pass_command (struct pop3_command * command, struct client * client) {
    
    if (client->user_auth) {

        FILE * file = fopen("users.txt", "r");
        if (file == NULL) {
            return FILE_ERR;
        }
        char buffer[BUFSIZE + 1] = {0};
        int arg_length = strlen(command->argument);
        
        while (fgets(buffer, BUFSIZE, file) != NULL) {
            int flag = 1;
            int i, j=0;

            while (buffer[j] != 0 && buffer[j++] != ',');

            for(i=0; buffer[j]!=0 && buffer[j]!='\n' && command->argument[i]!= 0 && flag && i<arg_length && j<BUFSIZE; i++, j++) {
                if (command->argument[i] != buffer[j]) {
                    flag = 0;
                }
            }

            if (flag && buffer[j] == '\n' && i==arg_length) {
                return_status ret = fill_mail(client);
                if (ret == OK_STATUS) {
                    client->state = TRANSACTION;
                    client->available_commands = transaction_command;
                    client->available_commands_count = transaction_command_count;
                    client->available_commands_functions = transaction_command_function;
                    
                    current = first_logged_in_user;
                    logged_in * aux = calloc(1, sizeof(logged_in));
                    aux->username = client->username;

                    if (current == NULL)
                    {
                        first_logged_in_user = aux;
                    }else{
                        while (current->next != NULL){
                            current = current->next;
                        }
                        current->next= aux;
                    }

                    return suscribe_ok(client);
                } else {
                    client->user_auth = false;
                    free(client->username);
                    client->username = NULL;
                    return ret;
                }
            }
        }
        client->user_auth = false;
        free(client->username);
        client->username = NULL;
        return suscribe_err(client);
    } else {
        return suscribe_err(client);
    }
}

return_status quit_auth_command (struct pop3_command * command, struct client * client) {

    client->state = CLOSING;
    return suscribe_ok(client);
}

return_status quit_command (struct pop3_command * command, struct client * client) {
    struct mail_file * current_mail = client->first_mail;

    while(current_mail != NULL) {
        if (current_mail->to_delete == 1) {
            remove(current_mail->file_name);
        }
        current_mail = current_mail->next;
    }


    remove_logged_user(client);


    client->state = CLOSING;
    return suscribe_ok(client);
}

void remove_logged_user(struct client * client){
    current = first_logged_in_user;

    if(client->username != NULL && current!=NULL && current->username != NULL){
        if(strcmp(current->username, client->username) == 0 ){
            first_logged_in_user = current->next;
            free(current);
        }else{
            int flag = 0;
            while (current->next != NULL && !flag) {
                if (strcmp(current->next->username, client->username) == 0)
                {
                    flag = 1;
                    current->next = current->next->next;
                    free(current->next);
                }else{
                    current = current->next;
                }
            }
        }
    }
}

return_status stat_command (struct pop3_command * command, struct client * client) {
    int file_count = 0;
    off_t dir_size = 0;
    struct stat st;
    
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

    if (sprintf(client->buffers.write, "+OK %d %lo\r\n", file_count, dir_size) <= 0) {
        return STRING_COPY;
    }
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    return OK_STATUS;
}

return_status list_command (struct pop3_command * command, struct client * client) {
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
                        return OK_STATUS;
                    }
                    not_deleted++;
                }
            current = current->next;
        }

        sprintf(client->buffers.write, "-ERR no such message, only %d messages in mail\r\n", not_deleted);\
        client->buffers.write_size = strlen(client->buffers.write);
        suscribe_write(client);
    }else{
        client->current_mail = current;
        fill_list_command(client);
    }
    
    return OK_STATUS;
}

void fill_list_command(struct client * client)
{
    struct stat st;
    struct mail_file * current = client->current_mail;
    char buffer[BUFSIZE] = {0};

    while (current != NULL) {
        if (current->to_delete == 0) {
            
            if(client->buffers.write_size+strlen(buffer) > BUFSIZE){
                client->state = WRITING_LIST;
                client->current_mail = current;
                return;
            }else if (stat(current->file_name, &st) >= 0) {
                sprintf(buffer, "+OK %d %lo\r\n", current->id, st.st_size);
                strcat(client->buffers.write,buffer);
                client->buffers.write_size += strlen(client->buffers.write);
                suscribe_write(client);
            }else if (errno == ENOENT) {
                log(ERROR, "%s", "Could not access file");
            } else {
                log(ERROR, "%d", errno);
            }
        }
        current = current->next;
    }

    if(client->buffers.write_size+strlen(".\r\n") <= BUFSIZE){
        strcat(client->buffers.write,".\r\n");
        client->buffers.write_size += strlen(client->buffers.write);
        suscribe_write(client);
        client->state=TRANSACTION;
    }


    return;
}

return_status retr_command (struct pop3_command * command, struct client * client) {
    char *end = 0;
    const long sl = strtol(command->argument, &end, 10);
    struct mail_file * current = client->first_mail;
    FILE * file;
    int mail_fd;
    int bytes_read = 0;

    struct stat st;

    while(current != NULL) {
        if (current->id == sl && current->to_delete == 0) {
            if ((mail_fd = open(current->file_name, O_RDONLY | O_NONBLOCK)) < 0 || stat(current->file_name, &st) < 0) {
                if (mail_fd >= 0) {
                    close(mail_fd);
                }
                return FILE_ERR;
            } else {
                client->mail_fd = mail_fd;
                sprintf(client->buffers.write, "+OK %lo Octets\r\n", st.st_size);
                client->buffers.write_size = strlen(client->buffers.write);
                suscribe_write(client);
                client->state = WRITING_MAIL;
                return OK_STATUS;
            }
        }
        current = current->next;
    }

    return suscribe_err(client);
}

int read_mail(struct client * client) {
    int bytes_read = read(client->mail_fd, client->buffers.write, BUFSIZE);
    if (bytes_read < 0) {
        client->state = TRANSACTION;
        client->interest = WRITE;
        close(client->mail_fd);
    } else if (bytes_read == 0) {
        client->state = TRANSACTION;
        sprintf(client->buffers.write, ".\r\n");
        client->buffers.write_size = 3;
        suscribe_write(client);
        close(client->mail_fd);
    } else {
        client->buffers.write_size = bytes_read;
        suscribe_write(client);
    }
    return bytes_read;
}

return_status dele_command (struct pop3_command * command, struct client * client) {
    char *end = 0;
    const long sl = strtol(command->argument, &end, 10);
    struct mail_file * current = client->first_mail;
    int i = 1;

    while(current != NULL) {
        if (i == sl && current->to_delete == 0) {
            current->to_delete = 1;
            return suscribe_ok(client);
        }
        current = current->next;
        i++;
    }

    return suscribe_err(client);
}

return_status noop_command (struct pop3_command * command, struct client * client) {
    return suscribe_ok(client);
}

return_status rset_command (struct pop3_command * command, struct client * client) {
    struct mail_file * current = client->first_mail;

    while(current != NULL) {
        current->to_delete = 0;
        current = current->next;
    }

    return suscribe_ok(client);
}

return_status historical_command (struct pop3_command * command, struct client * client) {
    sprintf(client->buffers.write, "+OK Historical connections: %d\r\n", client->metricas->historical_connections);
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    return OK_STATUS;
}

return_status concurrent_command (struct pop3_command * command, struct client * client) {
    sprintf(client->buffers.write, "+OK Concurrent connections: %d\r\n", client->metricas->concurrent_connections);
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    return OK_STATUS;
}

return_status bytes_sent_command (struct pop3_command * command, struct client * client) {
    sprintf(client->buffers.write, "+OK Bytes sent: %d\r\n", client->metricas->bytes_sent);
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    return OK_STATUS;
}

return_status monitor_login_command (struct pop3_command * command, struct client * client) {
    if (strcmp(command->argument, MONITOR_LOGIN_TOKEN)) {
        client->user_auth = true;
    }
    client->available_commands = monitor_commands;
    client->available_commands_count = monitor_commands_count;
    client->available_commands_functions = monitor_command_function;

    sprintf(client->buffers.write, "+OK You are monitoring\r\n");
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);

    return OK_STATUS;
}

return_status rmus_command (struct pop3_command * command, struct client * client) {
    if (command->argument[0] == '\0')
    {
        return suscribe_err(client);
    }

    current = first_logged_in_user;
    while (current != NULL)
    {
        if(strcmp(current->username, command->argument) == 0){
            sprintf(client->buffers.write, "-ERR user is logged in\r\n");
            client->buffers.write_size = strlen(client->buffers.write);
            suscribe_write(client);
            return OK_STATUS;
        }

        current = current->next;
    }


    FILE * file = fopen("users.txt", "r");
    if (file == NULL) {
        return FILE_ERR;
    }
    char buffer[BUFSIZE + 1] = {0};
    int arg_length = strlen(command->argument);
    int line = 1;

    if (command->argument[0] == 0) {
        return suscribe_err(client);
    }

    while (fgets(buffer, BUFSIZE, file) != NULL) {
        int flag = 1;
        int i;
        for(i=0; buffer[i]!=0 && command->argument[i]!= 0 && flag && i<arg_length &&i<BUFSIZE; i++) {
            if (command->argument[i] != buffer[i]) {
                flag = 0;
            }
        }

        if (flag && buffer[i] == ',' && i == arg_length) {
            FILE *temp;

            temp = fopen("delete.tmp", "w");
            if (temp == NULL){
                fclose(file);
                return FILE_ERR;
            }


            rewind(file);

            
            int count = 1;
            while ((fgets(buffer, BUFSIZE, file)) != NULL){
                if (line != count)
                    fputs(buffer, temp);
                count++;
            }

            fclose(file);
            fclose(temp);

            remove("users.txt");
            rename("delete.tmp", "users.txt");

            sprintf(client->buffers.write, "+OK user %s removed succesfully\r\n", command->argument);
            client->buffers.write_size = strlen(client->buffers.write);
            suscribe_write(client);
            return OK_STATUS;
        }
        line++;
    }

    fclose(file);

    sprintf(client->buffers.write, "-ERR user not found\r\n");
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);
    return OK_STATUS;
}

return_status adus_command (struct pop3_command * command, struct client * client) {
    if (command->argument[0] == '\0')
    {
        return suscribe_err(client);
    }

    int commas = 0;
    int i = 0;
    while (command->argument[i] != '\0')
    {
        if (command->argument[i] == ',')
        {
            commas++;
        }
        i++;
    }
    
    if (commas != 1 || command->argument[0] == ',' || command->argument[i-1] == ',')
    {
        sprintf(client->buffers.write, "-ERR invalid format\r\n");
        client->buffers.write_size = strlen(client->buffers.write);
        suscribe_write(client);
        return OK_STATUS;
    }

    FILE * file = fopen("users.txt", "r");
    if (file == NULL) {
        return FILE_ERR;
    }
    char buffer[BUFSIZE + 1] = {0};
    int arg_length = strlen(command->argument);


    if (command->argument[0] == 0) {
        return suscribe_err(client);
    }

    while (fgets(buffer, BUFSIZE, file) != NULL) {
        int flag = 1;
        i =0;
        for(i=0; buffer[i]!=0 && command->argument[i]!= 0 && flag && i<arg_length &&i<BUFSIZE; i++) {
            if (command->argument[i] != buffer[i]) {
                flag = 0;
            }
        }

        if (flag && buffer[i] == ',' && i == arg_length) {
            fclose(file);
            sprintf(client->buffers.write, "-ERR user %s already exists\r\n", command->argument);
            client->buffers.write_size = strlen(client->buffers.write);
            suscribe_write(client);
            return OK_STATUS;
        }
    }
    fclose(file);


    

    file = fopen("users.txt", "a");
    fprintf(file,"\n%s",command->argument);
    fclose(file);

    sprintf(client->buffers.write, "+OK user added sucesfully\r\n");
    client->buffers.write_size = strlen(client->buffers.write);
    suscribe_write(client);

    return OK_STATUS;
}