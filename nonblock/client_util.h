#ifndef _CLIENT_UTIL_H_
#define _CLIENT_UTIL_H_

struct client;

int fill_mail (struct client * client);
void free_client (struct client * client);

#endif