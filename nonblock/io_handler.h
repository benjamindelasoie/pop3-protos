#ifndef _I_O_HANDLER_H_
#define _I_O_HANDLER_H_

#include <sys/time.h>
#include "errors.h"

#define MAX_CLIENTS FD_SETSIZE
#define INVALID_FD(fd)  ((fd) < 0 || (fd) >= MAX_CLIENTS)

typedef struct select_info {
    int max_fd;

    fd_set readfds;
    fd_set writefds;

} select_info;

struct client;

int handle_write (struct client * client);
int handle_read (struct client * client);
void suscribe_read (struct client * client);
void suscribe_write (struct client * client);
return_status suscribe_err (struct client * client);
return_status suscribe_ok (struct client * client);
void recieve_flush(struct client * client);

#endif