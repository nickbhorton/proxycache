#include "connection.h"
#include "http.h"
#include "md5.h"
#include "utils.h"

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

// for inet_ntop
#include <arpa/inet.h>
#include <netinet/in.h>

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

static int get_file_from_origin(const StringView proxy_request, const Url* url, char filename[39]) {
    int connection_fd = tcp_connect(&url->domain, url->port, false);
    if (connection_fd < 0) {
        printf("could not connect to server\n");
        return -1;
    }

    int bytes_sent = pc_send(connection_fd, proxy_request.data, proxy_request.length);
    if (bytes_sent != (int)proxy_request.length) {
        printf("did not send entire proxy request request\n");
        close(connection_fd);
        return -1;
    }

    char rbuffer[PC_BUFFER_SIZE];
    int bytes_recv = pc_recv(connection_fd, rbuffer, PC_BUFFER_SIZE);
    if (bytes_recv <= 0) {
        printf("error with pc_recv\n");
        close(connection_fd);
        return -1;
    }

    // 1. break it down into http response and file
    StringView http_response_pass[2] = {};
    int rv = sv_split_n(http_response_pass, 2, rbuffer, bytes_recv, "\r\n\r\n", true);
    // TODO: don't fail here pc_recv again
    if (rv != 2) {
        printf("response did not have a header file divider\n");
        close(connection_fd);
        return -1;
    }
    int cached_file_size = http_response_pass[0].length + 4;

    // 2. find content length and make sure we get whole file from origin server
    StringView http_headers_pass[32] = {};
    int header_count = sv_split_n(
        http_headers_pass, 32, http_response_pass[0].data, http_response_pass[0].length, "\r\n",
        true
    );
    for (int i = 0; i < header_count; i++) {
        StringView http_header[2] = {};
        rv = sv_split_n(
            http_header, 2, http_headers_pass[i].data, http_headers_pass[i].length, ":", true
        );
        if (rv < 2) {
            continue;
        }
        const char* content_length_header = "Content-Length";
        const StringView content_length_sc = {
            .data = content_length_header, .length = strlen(content_length_header)
        };
        if (sv_cmp(http_header[0], content_length_sc, false)) {
            sv_strip(&http_header[1]);
            static char int_buffer[256];
            memcpy(int_buffer, http_header[1].data, http_header[1].length);
            int_buffer[http_header[1].length] = '\0';
            cached_file_size += atoi(int_buffer);
        }
    }
    printf("file_size: %d\n", cached_file_size);

    // 3. write file
    FILE* fptr;
    fptr = fopen(filename, "wb");
    if (fptr == NULL) {
        perror("fopen");
        close(connection_fd);
        return -1;
    }
    size_t written = 0;
    while (written < cached_file_size) {
        size_t current_written = fwrite(rbuffer, sizeof(char), bytes_recv, fptr);
        written += current_written;
        bytes_recv = pc_recv(connection_fd, rbuffer, PC_BUFFER_SIZE);
        if (bytes_recv <= 0) {
            break;
        }
    }
    if (written != cached_file_size) {
        perror("fwrite");
        close(connection_fd);
        fclose(fptr);
        return -1;
    }

    // clean up
    fclose(fptr);
    close(connection_fd);

    return 0;
}

int pc_get_file(const StringView proxy_request, const Url* url, size_t* file_size_o) {
    char filename[39];
    get_filename(url, filename);

    // try to open file if its in cache
    int rv = open(filename, O_RDONLY);
    if (rv > 0) {
        printf("cache hit\n");
        *file_size_o = get_filesize(filename);
        return rv;
    }

    rv = get_file_from_origin(proxy_request, url, filename);
    if (rv < 0) {
        return rv;
    }
    *file_size_o = get_filesize(filename);
    return open(filename, O_RDONLY);
}

void pc_handle_connection(Connection* c) {
    /*
    // printing the address of connection, remove eventually
    char addr_string[128] = {};
    inet_ntop(
        c->address.address.ss_family,
        (struct sockaddr*)&(((struct sockaddr_in*)&c->address.address)->sin_addr), addr_string,
        sizeof(addr_string)
    );
    printf("%d: client connection %s\n", getpid(), addr_string);
    */

    // getting client request
    char rbuffer[PC_BUFFER_SIZE];
    char sbuffer[PC_BUFFER_SIZE];
    int bytes_recv = 0;
    if ((bytes_recv = pc_recv(c->fd, rbuffer, PC_BUFFER_SIZE)) <= 0) {
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
    if (!tcp_server_exists(&url.domain, url.port)) {
        const char* msg = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(c->fd, msg, strlen(msg), 0);
        return;
    }

    // get file
    size_t file_size;
    int requested_file_fd = pc_get_file(proxy_request, &url, &file_size);
    if (requested_file_fd < 0) {
        perror("pc_get_file");
        return;
    }

    // send file to client
    ssize_t bytes_sent = sendfile(c->fd, requested_file_fd, 0, file_size);
    assert(bytes_sent == file_size);
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
