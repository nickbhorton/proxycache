#include "http_client.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "string_view.h"
#include "tcp.h"

int cl_get_atomic(const StringView request, const Url* url, const StringView filename) {
    static char filename_buffer[256];
    memcpy(filename_buffer, filename.data, filename.length);
    filename_buffer[filename.length] = '\0';

    // try to exclusivly create the file
    int fd = open(filename_buffer, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
    if (fd > 0) {
        // write locked
        int rv = flock(fd, LOCK_EX);
        if (rv < 0) {
            // write locked failed
            return -10;
        }
        rv = cl_get(request, url, fd);
        if (rv < 0) {
            // TODO check error
            remove(filename_buffer);
            return rv;
        }
        // unlocked
        rv = flock(fd, LOCK_UN);
        if (rv < 0) {
            // unlocked failed
            return -11;
        }
        close(fd);
    } else if (errno != EEXIST) {
        return -12;
    }

    fd = open(filename_buffer, O_RDONLY, S_IRWXU);
    // block if another proccess has a lock
    if (flock(fd, LOCK_SH) < 0) {
        return -13;
    }
    if (flock(fd, LOCK_UN) < 0) {
        return -13;
    }
    return fd;
}

int cl_get(const StringView request, const Url* url, int fd_to_write) {
    int origin_sock = tcp_connect(&url->domain, url->port, false);
    if (origin_sock < 0) {
        // could not connect to server
        return -1;
    }

    int bytes_sent = 0;
    while (bytes_sent < request.length) {
        int rv = send(origin_sock, request.data, request.length, 0);
        if (rv < 0) {
            // send failed
            shutdown(origin_sock, 2);
            return -2;
        }
        bytes_sent += rv;
    }

    static char recv_buffer[PC_BUFFER_SIZE];
    int bytes_recv = recv(origin_sock, recv_buffer, PC_BUFFER_SIZE, 0);
    if (bytes_recv < 0) {
        // recv failed
        shutdown(origin_sock, 2);
        return -3;
    }

    StringView header_body_pass[2];
    int word_count = sv_split_n(header_body_pass, 2, recv_buffer, bytes_recv, "\r\n\r\n", true);
    if (word_count < 1) {
        // did not recv enough from server to find header body split point
        shutdown(origin_sock, 2);
        return -4;
    }

    ssize_t bytes_written = 0;
    if (word_count == 2) {
        bytes_written = write(fd_to_write, header_body_pass[1].data, header_body_pass[1].length);
        if (bytes_written != header_body_pass[1].length) {
            // did not write all file bytes from first recv
            shutdown(origin_sock, 2);
            return -6;
        }
    }

    // find content length and make sure we get whole file from origin server
    size_t content_length = 0;

    StringView http_headers_pass[64] = {};
    int header_count = sv_split_n(
        http_headers_pass, 64, header_body_pass[0].data, header_body_pass[0].length, "\r\n", true
    );
    for (int i = 0; i < header_count; i++) {
        StringView http_header[2] = {};
        int wc = sv_split_n(
            http_header, 2, http_headers_pass[i].data, http_headers_pass[i].length, ":", true
        );
        if (wc < 2) {
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
            content_length = atoi(int_buffer);
        }
    }
    if (content_length == 0) {
        // content length was not found in response
        shutdown(origin_sock, 2);
        return -7;
    }

    while (bytes_written < content_length) {
        bytes_recv = recv(origin_sock, recv_buffer, PC_BUFFER_SIZE, 0);
        if (bytes_recv < 0) {
            // recv failed for rest of file
            shutdown(origin_sock, 2);
            return -8;
        }
        ssize_t bytes_written_cur = write(fd_to_write, recv_buffer, bytes_recv);
        if (bytes_written_cur != bytes_recv) {
            // did not write all bytes from rest of file
            shutdown(origin_sock, 2);
            return -9;
        }
        bytes_written += bytes_written_cur;
    }

    shutdown(origin_sock, 2);
    return 0;
}
