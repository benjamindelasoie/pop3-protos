#ifndef _CLIENT_UTIL_H_
#define _CLIENT_UTIL_H_

#include "errors.h"

struct client;

return_status fill_mail (struct client * client);
void free_client (struct client * client);

#endif