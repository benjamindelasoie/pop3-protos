#include <sys/types.h>   // socket
#include <sys/socket.h>  // socket
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <string.h>

#include "socket_util.h"
#include "logger.h"

int setup_server_socket(unsigned port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        log(FATAL, "%s", "unable to create socket");
    }

    log(DEBUG, "Listening on TCP port %d\n", port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

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