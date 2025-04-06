#include "tcp.h"
#include "macros.h"
#include "string_view.h"

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fnmatch.h>
#include <netdb.h>
#include <netinet/in.h>
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

static bool is_blocked(const StringView* node, struct addrinfo* info) {
    // buffers to store strings to check against
    static char addr_string[INET6_ADDRSTRLEN];
    static char node_string[1024];
    // buffer to store the file
    static char blocklist[16384];
    static size_t blocklist_length = 0;
    static StringView blocklist_lines[512];
    static int blocklist_lines_count = 0;
    // read the file in the first time is_blocked is called
    static bool has_block_file = true;
    static bool blocked_file_is_init = false;
    if (!blocked_file_is_init) {
        blocked_file_is_init = true;
        FILE* fptr = fopen("./blocklist", "r");
        if (fptr == NULL) {
            has_block_file = false;
            return false;
        }
        blocklist_length = fread(blocklist, sizeof(char), 16384, fptr);
        blocklist_lines_count =
            sv_split_n(blocklist_lines, 512, blocklist, blocklist_length, "\n", false);
        // printf("blocked file length %zu, line_count %d\n", blocklist_length,
        // blocklist_lines_count);
    }

    // if there is no block file just return
    if (!has_block_file) {
        return false;
    }

    if (node != NULL) {
        memcpy(node_string, node->data, node->length);
        node_string[node->length] = '\0';
    } else {
        node_string[0] = '\0';
    }
    if (info->ai_family == AF_INET) {
        inet_ntop(
            info->ai_family, &((struct sockaddr_in*)(info->ai_addr))->sin_addr, addr_string,
            sizeof(addr_string)
        );
    } else {
        inet_ntop(
            info->ai_family, &((struct sockaddr_in6*)(info->ai_addr))->sin6_addr, addr_string,
            sizeof(addr_string)
        );
    }
    for (int i = 0; i < blocklist_lines_count; i++) {
        static char pattern[512];
        memcpy(pattern, blocklist_lines[i].data, blocklist_lines[i].length);
        pattern[blocklist_lines[i].length] = '\0';
        if (fnmatch(pattern, addr_string, 0) == 0) {
            // printf("blocked %s\n", addr_string);
            return true;
        }
        if (fnmatch(pattern, node_string, 0) == 0) {
            // printf("blocked %s\n", node_string);
            return true;
        }
    }
    return false;
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
    if (is_blocked(node, ai_ptr)) {
        return -2;
    }

    freeaddrinfo(ai_list);
    return socket_fd;
}
