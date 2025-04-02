#include "tcp.h"
#include "macros.h"

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static int tcp_address_info(const char* node, const char* service, struct addrinfo** ai_list) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (node == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    // this is going to block for a good few seconds if node does not exist
    // an artical online says that serious applications use libresolv
    int rv = getaddrinfo(node, service, &hints, ai_list);
    return rv;
}

int tcp_connect(const StringView* node, int16_t port, bool server) {
    struct addrinfo* ai_list = NULL;

    static char node_cstr[512];
    if (node != NULL) {
        snprintf(node_cstr, 512, "%.*s", (int)node->length, node->data);
    }

    static char port_cstr[128];
    snprintf(port_cstr, 128, "%d", port);

    int rv = tcp_address_info(node == NULL ? NULL : node_cstr, port_cstr, &ai_list);
    if (rv != 0) {
        DebugError("tcp_connect", gai_strerror(rv));
        return -1;
    }

    int socket_fd;
    struct addrinfo* ai_ptr;
    for (ai_ptr = ai_list; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
        socket_fd = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }

        if (server) {
            static const int yes = 1;
            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                DebugError("tcp_connect::setsockopt", strerror(errno));
                return -1;
            }
        }

        rv = server ? bind(socket_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen)
                    : connect(socket_fd, ai_ptr->ai_addr, ai_ptr->ai_addrlen);
        if (rv < 0) {
            continue;
        }
        break;
    }

    if (ai_ptr == NULL) {
        DebugError("tcp_connect", "failed to find server");
        freeaddrinfo(ai_list);
        return -1;
    }

    freeaddrinfo(ai_list);
    return socket_fd;
}
