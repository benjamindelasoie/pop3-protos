#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "client.h"
#include "client_util.h"

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
    if (client->username != NULL)
    {
        free(client->username);
    }
    

    struct mail_file * current = client->first_mail;
    struct mail_file * next;

    while (current != NULL) {
        next = current->next;
        free(current->file_name);
        free(current);
        current = next;
    }

    free(client);
}