#include <sys/socket.h>

#include "io_handler.h"
#include "logger.h"
#include "client.h"
#include "pop_commands.h"

int handle_read (struct client * client) {
    client->buffers.bytes_recieved = recv(client->fd, client->buffers.recieve, BUFSIZE, MSG_NOSIGNAL);
    if (client->buffers.bytes_recieved <= 0) {
        if (client->buffers.bytes_recieved == 0) {
            return 0;
        } else {
            //check errors
            return -1; 
        }
    }

    return_status ret = recieve_flush(client);
    return handle_error(client, ret);
}

int handle_write (struct client * client, struct metrics * metricas) {
    int bytes_sent = send(client->fd, &client->buffers.write[client->buffers.write_index], client->buffers.write_size, MSG_NOSIGNAL);
    if (bytes_sent <= 0) {
        return -1;
    }

    client->buffers.write_index += bytes_sent;
    client->buffers.write_size -= bytes_sent;
    metricas->bytes_sent += bytes_sent;

    if (client->buffers.write_size > 0) {
        suscribe_write(client);
    } else if(client->state == WRITING_LIST){
        fill_list_command(client);
    } else if (client->state == CLOSING) {
        return 0;
    } else if (client->state == WRITING_MAIL) {
        client->interest = READ_FILE;
    } else {
        for (size_t i = 0; i < BUFSIZE; i++)
        {
            client->buffers.write[i] = 0;
        }
        
        client->buffers.write_index = 0;
        client->buffers.write_size = 0;
        if (client->buffers.bytes_recieved > 0) {
            return_status ret = recieve_flush(client);
            return handle_error(client, ret);
        } else {
            suscribe_read(client);
        }
    }
    return 1;
}

int handle_error(struct client * client, return_status ret) {
    switch (ret) {
    case OK_STATUS:
        return 1;
    case MEMORY_ALOC: case FILE_ERR:
        if (suscribe_err(client) == STRING_COPY) {
            return -1;
        } else {
            return 1;
        }
    case RECV_ERR: case SEND_ERR: case STRING_COPY: 
        return -1;
    default:
        return 1;
    }
}

return_status suscribe_err (struct client * client) {
    int aux = sprintf(client->buffers.write, "-ERR\r\n");
    if (aux < 6) {
        return STRING_COPY;
    } else {
        client->buffers.write_size = 7;
        client->buffers.write_index = 0;
    }
    suscribe_write(client);
    return OK_STATUS;
}

return_status suscribe_ok (struct client * client) {
    int aux = sprintf(client->buffers.write, "+OK\r\n");
    if (aux < 5) {
        // log(DEBUG, "buffer: %s aux: %d", client->buffers.write, aux);
        //errors
        return STRING_COPY;
    } else {
        client->buffers.write_size = 6;
        client->buffers.write_index = 0;
    }
    suscribe_write(client);
    return OK_STATUS;
}

void suscribe_write (struct client * client) {
    client->interest = WRITE;
}

void suscribe_read (struct client * client) {
    client->interest = READ;
}

return_status recieve_flush(struct client * client) {
    int ready = read_line(&client->buffers);

    if (ready == 1) {
        pop3_command * command = fill_command(client->buffers.parser);
        if (command != NULL) {
            int ok;
            if ((ok = parse(command, client)) < 0) {
                free (command);
                return suscribe_err(client);
            }else {
                return_status ret;
                ret = (*client->available_commands_functions[ok])(command, client);
                free(command);
                return ret;
                // suscribe_ok(client);
            }
        } else {
            return suscribe_err(client);
        }
    } else {
        suscribe_read(client);
        return OK_STATUS;
    }
}