#include "utils.h"
#include "macros.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int http_connect(const char* node) {
    int rv;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* ai_list;
    rv = getaddrinfo(node, "http", &hints, &ai_list);
    if (rv != 0) {
        DebugError("http_connect::getaddrinfo", gai_strerror(rv));
        return -1;
    }

    int socket_fd;
    struct addrinfo* ai_ptr;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
        socket_fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (socket_fd < 0) {
            DebugError("http_connect::socket", strerror(errno));
            continue;
        }

        rv = connect(socket_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
        if (rv < 0) {
            DebugError("http_connect::connect", strerror(errno));
            continue;
        }
        break;
    }

    if (ai_ptr == NULL) {
        DebugError("http_connect", "failed to find server");
        freeaddrinfo(ai_list);
        return -1;
    }

    freeaddrinfo(ai_list);
    return socket_fd;
}
