#include "connection.h"
#include "http.h"
#include "utils.h"

#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// for inet_ntop
#include <arpa/inet.h>
#include <netinet/in.h>

#define PC_TIMEOUT_MS 1000
#define PC_BUFFER_SIZE 4096

int pc_recv(Connection* c, char* recv_buffer, size_t buffer_size) {
    static struct pollfd pfd[1];
    static bool init_pfd = false;
    if (!init_pfd) {
        pfd[0].events = POLLIN;
    }
    pfd[0].fd = c->fd;
    int num_events = poll(pfd, 1, PC_TIMEOUT_MS);
    if (num_events == 0) {
        return 0;
    } else if (pfd[0].revents & POLLIN) {
        int bytes_recv = recv(c->fd, recv_buffer, buffer_size, 0);
        return bytes_recv;
    }
    return -1;
}

void pc_handle_connection(Connection* c) {
    // printing the address of connection
    char addr_string[128] = {};
    inet_ntop(
        c->address.address.ss_family,
        (struct sockaddr*)&(((struct sockaddr_in*)&c->address.address)->sin_addr), addr_string,
        sizeof(addr_string)
    );
    printf("%d: client connection %s\n", getpid(), addr_string);

    // getting request
    char rbuffer[PC_BUFFER_SIZE];
    char sbuffer[PC_BUFFER_SIZE];
    Url url = {};
    int bytes_recv = 0;
    if ((bytes_recv = pc_recv(c, rbuffer, PC_BUFFER_SIZE)) > 0) {
        int proxy_request_length = to_proxy_request(rbuffer, bytes_recv, sbuffer, &url);
        if (proxy_request_length < 0) {
            const char* msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
            send(c->fd, msg, strlen(msg), 0);
            return;
        }

        // validate that the server exists
        char domain[256];
        memcpy(domain, url.domain.data, url.domain.length);
        domain[url.domain.length] = '\0';
        if (!http_server_exists(domain)) {
            const char* msg = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(c->fd, msg, strlen(msg), 0);
            return;
        }

        // print stuff
        printf(
            "url: %.*s:%d/%.*s\n", (int)url.domain.length, url.domain.data, url.port,
            (int)url.path.length, url.path.data
        );
        printf("proxy request:\n%.*s\n", proxy_request_length, sbuffer);
    }
    return;
}

Connection pc_accept(int sockfd) {
    Connection result = {};
    result.address.address_length = sizeof(result.address.address);
    while (1) {
        int rv = accept(
            sockfd, (struct sockaddr*)&result.address.address, &result.address.address_length
        );
        if (rv < 0) {
            // indicates caller made a mistake
            // this should never happen in normal operation
            if (errno == EBADF || errno == EINVAL || errno == ENOTSOCK || errno == EOPNOTSUPP) {
                perror("pc_accept");
                result.fd = -1;
                exit(1);
                return result;
                // indicates that something went wrong and accept should be called again
            } else if (errno == ECONNABORTED || errno == EINTR) {
                continue;
            } else {
                perror("pc_accept");
                fprintf(stderr, "Critical error in pc_accept programming exiting\n");
                fflush(stderr);
                exit(EXIT_FAILURE);
            }
        }
        result.fd = rv;
        break;
    }
    return result;
}
