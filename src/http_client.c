#include "http_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "string_view.h"
#include "tcp.h"

int cl_get(const StringView request, const Url* url, const StringView filename_to_write) {
    int fd = tcp_connect(&url->domain, url->port, false);
    if (fd < 0) {
        // could not connect to server
        return -1;
    }

    int bytes_sent = 0;
    while (bytes_sent < request.length) {
        int rv = send(fd, request.data, request.length, 0);
        if (rv < 0) {
            // send failed
            shutdown(fd, 2);
            return -2;
        }
        bytes_sent += rv;
    }

    static char recv_buffer[PC_BUFFER_SIZE];
    int bytes_recv = recv(fd, recv_buffer, PC_BUFFER_SIZE, 0);
    if (bytes_recv < 0) {
        // recv failed
        shutdown(fd, 2);
        return -3;
    }

    StringView header_body_pass[2];
    int word_count = sv_split_n(header_body_pass, 2, recv_buffer, bytes_recv, "\r\n\r\n", true);
    if (word_count < 1) {
        // did not recv enough from server to find header body split point
        shutdown(fd, 2);
        return -4;
    }

    static char filename_buffer[256];
    memcpy(filename_buffer, filename_to_write.data, filename_to_write.length);
    filename_buffer[filename_to_write.length] = '\0';
    FILE* fptr = fopen(filename_buffer, "w");
    if (fptr == NULL) {
        // could not open file
        shutdown(fd, 2);
        return -5;
    }

    size_t bytes_written = 0;
    if (word_count == 2) {
        bytes_written =
            fwrite(header_body_pass[1].data, sizeof(char), header_body_pass[1].length, fptr);
        if (bytes_written != header_body_pass[1].length) {
            // did not write all file bytes from first recv
            fclose(fptr);
            shutdown(fd, 2);
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
        fclose(fptr);
        shutdown(fd, 2);
        return -7;
    }

    while (bytes_written < content_length) {
        bytes_recv = recv(fd, recv_buffer, PC_BUFFER_SIZE, 0);
        if (bytes_recv < 0) {
            // recv failed for rest of file
            fclose(fptr);
            shutdown(fd, 2);
            return -8;
        }
        size_t bytes_written_cur = fwrite(recv_buffer, sizeof(char), bytes_recv, fptr);
        if (bytes_written_cur != bytes_recv) {
            // did not write all bytes from rest of file
            fclose(fptr);
            shutdown(fd, 2);
            return -9;
        }
        bytes_written += bytes_written_cur;
    }

    fclose(fptr);
    shutdown(fd, 2);
    return 0;
}
