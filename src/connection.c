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

int pc_send(Connection* c, const char* send_buffer, size_t buffer_size) {
    // TODO: ensure whole request is sent
    return send(c->fd, send_buffer, buffer_size, 0);
}

int pc_get_file(const StringView proxy_request, const Url* url) {
    /*
    // print stuff
    printf(
        "url: %.*s:%d/%.*s\n", (int)url->domain.length, url->domain.data, url->port,
        (int)url->path.length, url->path.data
    );
    printf("proxy request:\n%.*s\n", (int)proxy_request.length, proxy_request.data);
    */

    // TODO: make http_server_exists use StringView
    char domain[256];
    memcpy(domain, url->domain.data, url->domain.length);
    domain[url->domain.length] = '\0';
    // TODO: make sure we use the right port to in http_connect
    // ENDED SESSOIN HERE
    // int connection_fd = http_connect(domain);
    return 0;
}

void pc_handle_connection(Connection* c) {
    // printing the address of connection, remove eventually
    char addr_string[128] = {};
    inet_ntop(
        c->address.address.ss_family,
        (struct sockaddr*)&(((struct sockaddr_in*)&c->address.address)->sin_addr), addr_string,
        sizeof(addr_string)
    );
    printf("%d: client connection %s\n", getpid(), addr_string);

    // getting client request
    char rbuffer[PC_BUFFER_SIZE];
    char sbuffer[PC_BUFFER_SIZE];
    int bytes_recv = 0;
    if ((bytes_recv = pc_recv(c, rbuffer, PC_BUFFER_SIZE)) <= 0) {
        return;
    }

    // validate the client request can be transmuted to proxy request
    StringView proxy_request = {};
    Url url = {};
    int proxy_request_length = to_proxy_request(rbuffer, bytes_recv, sbuffer, &url);
    if (proxy_request_length < 0) {
        const char* msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(c->fd, msg, strlen(msg), 0);
        return;
    }
    proxy_request.length = (size_t)proxy_request_length;
    proxy_request.data = sbuffer;

    // validate that the origin server in request exists
    // TODO: make http_server_exists use StringView
    char domain[256];
    memcpy(domain, url.domain.data, url.domain.length);
    domain[url.domain.length] = '\0';
    if (!http_server_exists(domain)) {
        const char* msg = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(c->fd, msg, strlen(msg), 0);
        return;
    }

    // get file from origin server
    [[maybe_unused]] int requested_file_fd = pc_get_file(proxy_request, &url);
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
