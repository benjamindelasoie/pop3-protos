#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <string.h>

#include "socket_util.h"
#include "logger.h"

int setup_server_socket(unsigned port) {
    struct sockaddr_in6 addr;

    const int server = socket(AF_INET6, SOCK_STREAM, 0);
    if(server < 0) {
        log(FATAL, "%s", "unable to create socket");
    }

    log(DEBUG, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        log(FATAL, "%s", "setsockopt failed");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family      = AF_INET6;
    addr.sin6_addr        = in6addr_any;
    addr.sin6_port        = htons(port);

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        log(FATAL, "%s", "unable to bind socket");
    }

    if (listen(server, 20) < 0) {
        log(FATAL, "%s", "unable to listen");
    }

    return server;
}

int set_socket_nonblock (const int fd) {
    int flags = fcntl(fd, F_GETFD, 0);
    if(flags == -1) {
        return -1;
    } else {
        if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            return -1;
        }
    }
    return 0;
}