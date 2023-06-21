#ifndef _SOCKET_UTIL_H_
#define _SOCKET_UTIL_H_

#include "errors.h"

int setup_server_socket(unsigned port);
int set_socket_nonblock (const int fd);

#endif