#include "connection.h"
#include "http.h"
#include "http_client.h"
#include "md5.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

int pc_recv(int fd, char* recv_buffer, size_t buffer_size) {
    static struct pollfd pfd[1];
    static bool init_pfd = false;
    if (!init_pfd) {
        pfd[0].events = POLLIN;
    }
    pfd[0].fd = fd;
    int num_events = poll(pfd, 1, PC_TIMEOUT_MS);
    if (num_events == 0) {
        return 0;
    } else if (pfd[0].revents & POLLIN) {
        int bytes_recv = recv(fd, recv_buffer, buffer_size, 0);
        return bytes_recv;
    }
    return -1;
}

int pc_send(int fd, const char* send_buffer, size_t buffer_size) {
    // TODO: ensure whole request is sent
    int bytes_sent = send(fd, send_buffer, buffer_size, 0);
    assert(bytes_sent == buffer_size);
    return bytes_sent;
}

static ssize_t get_filesize(const char filename[39]) {
    struct stat st;
    int rv = stat(filename, &st);
    if (rv < 0) {
        perror("stat");
        return rv;
    }
    return st.st_size;
}

void get_url_hash(const Url* url, uint8_t hash[16]) {
    static char hash_buffer[512];
    snprintf(
        hash_buffer, 512, "%.*s/%.*s", (int)url->domain.length, url->domain.data,
        (int)url->path.length, url->path.data
    );
    md5String(hash_buffer, hash);
}

static void get_filename(const Url* url, char filename_o[39]) {
    static char hash_buffer[512];
    static uint8_t file_hash[16];
    snprintf(
        hash_buffer, 512, "%.*s/%.*s", (int)url->domain.length, url->domain.data,
        (int)url->path.length, url->path.data
    );
    printf("%s\n", hash_buffer);
    md5String(hash_buffer, file_hash);
    snprintf(
        filename_o, 39, "cache/%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        file_hash[0], file_hash[1], file_hash[2], file_hash[3], file_hash[4], file_hash[5],
        file_hash[6], file_hash[7], file_hash[8], file_hash[9], file_hash[10], file_hash[11],
        file_hash[12], file_hash[13], file_hash[14], file_hash[15]
    );
}

int pc_handle_connection(Connection* c) {
    // getting client request
    char rbuffer[PC_BUFFER_SIZE];
    char sbuffer[PC_BUFFER_SIZE];
    int bytes_recv = 0;
    if ((bytes_recv = pc_recv(c->fd, rbuffer, PC_BUFFER_SIZE)) <= 0) {
        // recv error or timed out
        return -1;
    }

    // validate the client request can be transmuted to proxy request
    StringView proxy_request = {};
    Url url = {};
    int proxy_request_length = to_proxy_request(rbuffer, bytes_recv, sbuffer, &url);
    if (proxy_request_length < 0) {
        const char* msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(c->fd, msg, strlen(msg), 0);
        // proxy request transmutation has error
        return -2;
    }
    proxy_request.length = (size_t)proxy_request_length;
    proxy_request.data = sbuffer;

    // get file
    char filename[39];
    get_filename(&url, filename);
    StringView filename_sv = {.data = filename, .length = 38};
    int requested_file_fd = cl_get_atomic(proxy_request, &url, filename_sv);
    if (requested_file_fd < 0) {
        return -3;
    }
    ssize_t file_size = get_filesize(filename);
    if (file_size <= 0) {
        return -4;
    }

    // send file (header is at top of file) to client
    ssize_t bytes_sent = sendfile(c->fd, requested_file_fd, 0, file_size);
    if (bytes_sent != file_size) {
        // did not send enough bytes to client
        return -5;
    }
    return 0;
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
