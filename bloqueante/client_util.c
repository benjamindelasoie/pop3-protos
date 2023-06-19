#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <dirent.h>
// #include <sys/stat.h>

#include "parser.h"
#include "clients.h"
#include "logger.h"
#include "pop_commands.h"

void * handleClient (void * args) {
    int * client_fd = (int *) args;

    struct client client;
    memset(&client, 0, sizeof(client));
    client.fd = *client_fd;
    client.mail_directory = mail_directory;
    client.state = GREETING;
    log(DEBUG, "%s", mail_directory);

    send(client.fd, "+OK SERVER READY\r\n",19,0);

    client.state = AUTHORIZATION;
    client.available_commands = authorization_command;
    client.available_commands_count = authorization_command_count;
    client.available_commands_functions = authorization_command_function;

    ssize_t bytes_recieved = 0;
    int ok = 1;
    while (ok != 0 && (bytes_recieved = recv(client.fd, client.buffers.recieve, BUFSIZE, 0)) > 0) {
        while (ok != 0 && bytes_recieved > 0) {
            // log(DEBUG, "%ld bytes recieved", bytes_recieved);
            // log(DEBUG, "buffer: %s", client_buffers.recieve);
            int ready = read_line(&client.buffers);
            bytes_recieved -= client.buffers.parser_index;
            // log(DEBUG, "parse_buffer: %s", client_buffers.parser);
            // log(DEBUG, "ready: %d", ready);
            if (ready == 1) {
                pop3_command * command = fill_command(client.buffers.parser);
                client.buffers.parser_index = 0;
                ssize_t num_bytes_sent;
                if (command != NULL) {
                    if ((ok = parse(command, &client)) < 0) {
                        free (command);
                        num_bytes_sent = send(client.fd, "-ERR\r\n", 7, 0);
                    }else {
                        (*client.available_commands_functions[ok])(command, &client);
                        free(command);
                    }
                } else {
                    num_bytes_sent = send(client.fd, "-ERR\r\n", 7, 0);
                }
                if (num_bytes_sent < 0) {
                    ok = 0;
                    log(ERROR, "%s", "send() failed");
                }
            }
        }
        client.buffers.recieve_index = 0;
    }

    if (ok == 0){
        free_client(&client);
        close(client.fd);
    }else if (bytes_recieved < 0) {
        log(ERROR, "%s", "recv() failed");
    } else {
        // log(DEBUG, "closing client %d", client.fd);
        free_client(&client);
        close(client.fd);
    }

    return 0;
}

//fills up paser emptying reciever
ssize_t read_line(struct buffers * buffers) {
    int crlf = 0;
    for ( ; buffers->recieve[buffers->recieve_index] != 0 && buffers->parser_index<BUFSIZE && buffers->recieve_index<BUFSIZE; 
        buffers->parser_index++, buffers->recieve_index++) {
        
        char c = buffers->recieve[buffers->recieve_index];
        buffers->parser[buffers->parser_index] = c;
        buffers->recieve[buffers->recieve_index] = 0;
        
        if( c == '\r' ) {
            crlf = 1;
        } else if (c == '\n') {
            if (crlf) {
                buffers->recieve_index++;
                buffers->parser_index++;
                return 1;
            } else {
                crlf = 0;
            }
        } else {
            crlf = 0;
        }
    }
    if (buffers->recieve_index >= BUFSIZE || buffers->recieve[buffers->recieve_index] == 0) {
        buffers->recieve_index = 0;
    }
    if (buffers->parser_index == BUFSIZE) {
        return 1;
    }
    return 0;
}

void * set_mail_directory(void * args) {
    char * directory = (char *) args;
    log(DEBUG, "%s", directory);
    int length = strlen(directory);
    if ((mail_directory = calloc(1, length + 1)) == NULL) {
        log(ERROR, "%s", "Could Not Calloc mail directory");
        pthread_exit((int *)-1);
    }
    if (sprintf(mail_directory, "%s", directory) < length ) {
        free(mail_directory);
        log(ERROR, "%s", "Could Not copy mail directory");
        pthread_exit((int *)-1);
    } 
    pthread_exit(0);
}

int fill_mail (struct client * client) {
    int direc_lenght = strlen(client->mail_directory) + 1 + strlen(client->username) + strlen("/cur");
    char * maildir = calloc(1, direc_lenght + 1);
    if (sprintf(maildir, "%s/%s/cur", client->mail_directory, client->username) < direc_lenght) {
        free(maildir);
        return -1;
    }
    DIR * directory = opendir(maildir);
    struct dirent * file;
    struct mail_file * current = NULL;

    if (directory != NULL) {
        while ((file = readdir(directory)) != NULL) {
            if (strcmp(file->d_name,".") != 0 && strcmp(file->d_name,"..")!=0) {

                if (client->first_mail == NULL) {
                    if ((client->first_mail = calloc(1, sizeof(struct mail_file))) == NULL) {
                        return -1;
                    } else {
                        current = client->first_mail; 
                    }
                } else {
                    if ((current->next = calloc(1, sizeof(struct mail_file))) == NULL) {
                        return -1;
                    } else {
                        current = current->next;
                    }
                }

                int file_lenght = direc_lenght+1+strlen(file->d_name);
                if ((current->file_name = calloc(1, file_lenght+1)) == NULL) {
                    return -1;
                } else {
                    if (sprintf(current->file_name, "%s/%s", maildir, file->d_name) < file_lenght) {
                        free (current->file_name);
                        current->file_name = NULL;
                        return -1;
                    }
                }

            }
        }    
    }
    closedir(directory);
    free(maildir);
    return 0;

}

void free_client (struct client * client) {
    free(client->username);

    struct mail_file * current = client->first_mail;
    struct mail_file * next;

    while (current != NULL) {
        next = current->next;
        free(current->file_name);
        free(current);
        current = next;
    }
}