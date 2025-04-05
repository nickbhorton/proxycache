#include "http_client.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "connection.h"
#include "string_view.h"
#include "tcp.h"

#define CACHE_LIFETIME 10.0

int cl_get_atomic(const StringView request, const Url* url, const StringView filename) {
    static char filename_buffer[256];
    memcpy(filename_buffer, filename.data, filename.length);
    filename_buffer[filename.length] = '\0';

    // try to exclusivly create the file
    int fd;
create_file:
    fd = open(filename_buffer, O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
    if (fd > 0) {
        // lock
        int rv = flock(fd, LOCK_EX);
        if (rv < 0) {
            // write locked failed
            close(fd);
            if (remove(filename_buffer) < 0) {
                perror("remove");
            }
            return -20;
        }
        // get file from origin
        rv = cl_get(request, url, fd);
        if (rv < 0) {
            close(fd);
            if (remove(filename_buffer) < 0) {
                perror("remove");
            }
            return rv;
        }
        // unlock
        rv = flock(fd, LOCK_UN);
        if (rv < 0) {
            // unlocked failed
            close(fd);
            if (remove(filename_buffer) < 0) {
                perror("remove");
            }
            return -21;
        }
        close(fd);
    } else if (errno != EEXIST) {
        return -22;
    } else {
        printf("%s cache hit\n", filename_buffer);
    }

    //
    // check file age, remove and recreate if old
    //
    struct stat st = {};
    int rv = stat(filename_buffer, &st);
    if (rv < 0) {
        perror("stat");
        return -23;
    }
    time_t t_now = time(NULL);
    double file_age_seconds = difftime(t_now, st.st_mtime);
    if (CACHE_LIFETIME < file_age_seconds) {
        printf("%s is old\n", filename_buffer);
        if (remove(filename_buffer) < 0) {
            perror("remove");
            return -24;
        }
        goto create_file;
    }

    //
    // open the file in read mode to return
    //
    fd = open(filename_buffer, O_RDONLY);
    if (fd < 0) {
        return -25;
    }

    // block if another proccess has a lock
    if (flock(fd, LOCK_SH) < 0) {
        close(fd);
        return -26;
    }
    if (flock(fd, LOCK_UN) < 0) {
        close(fd);
        return -27;
    }

    // TODO if for some reason there is a critical error in the proccess that is required to get the
    // file from origin then we are still cooked
    return fd;
}

const char* err_msg_404 = "HTTP/1.1 404 Not Found\r\n\r\n";
const char* err_msg_500 = "HTTP/1.1 500 Internal Server Error\r\n\r\n";

// TODO fix this function. it is so long and bad
int cl_get(const StringView request, const Url* url, int fd_to_write) {
    ssize_t bytes_written = 0;
    //
    // establish a connection to the origin and send request
    //
    int origin_sock = tcp_connect(&url->domain, url->port, false);
    if (origin_sock < 0) {
        // 404 -> could not connect to server
        bytes_written = write(fd_to_write, err_msg_404, strlen(err_msg_404));
        if (bytes_written != strlen(err_msg_404)) {
            return -1;
        }
        return 0;
    }

    int bytes_sent = 0;
    while (bytes_sent < request.length) {
        int rv = send(origin_sock, request.data, request.length, 0);
        if (rv < 0) {
            shutdown(origin_sock, 2);
            // 500 -> send failed
            bytes_written = write(fd_to_write, err_msg_500, strlen(err_msg_500));
            if (bytes_written != strlen(err_msg_500)) {
                return -2;
            }
            return 0;
        }
        bytes_sent += rv;
    }

    static char recv_buffer[PC_BUFFER_SIZE];
    int bytes_recv = recv(origin_sock, recv_buffer, PC_BUFFER_SIZE, 0);
    if (bytes_recv < 0) {
        shutdown(origin_sock, 2);
        // 500 -> recv failed
        bytes_written = write(fd_to_write, err_msg_500, strlen(err_msg_500));
        if (bytes_written != strlen(err_msg_500)) {
            return -3;
        }
        return 0;
    }

    //
    // parse origin response
    //
    StringView header_body_pass[2];
    int header_body_wc = sv_split_n(header_body_pass, 2, recv_buffer, bytes_recv, "\r\n\r\n", true);
    if (header_body_wc < 1) {
        shutdown(origin_sock, 2);
        // 500 -> did not recv enough from server to find header body split point
        bytes_written = write(fd_to_write, err_msg_500, strlen(err_msg_500));
        if (bytes_written != strlen(err_msg_500)) {
            return -4;
        }
        return 0;
    }

    StringView http_headers_pass[64] = {};
    int header_count = sv_split_n(
        http_headers_pass, 64, header_body_pass[0].data, header_body_pass[0].length, "\r\n", true
    );
    if (header_count < 1) {
        shutdown(origin_sock, 2);
        // 500 -> origin gave a very strange response
        bytes_written = write(fd_to_write, err_msg_500, strlen(err_msg_500));
        if (bytes_written != strlen(err_msg_500)) {
            return -5;
        }
        return 0;
    }

    //
    // write file
    //
    // write top line to file
    bytes_written = write(fd_to_write, http_headers_pass[0].data, http_headers_pass[0].length);
    if (bytes_written != http_headers_pass[0].length) {
        shutdown(origin_sock, 2);
        return -6;
    }
    size_t cur_bytes_written = write(fd_to_write, "\r\n", 2);
    if (cur_bytes_written != 2) {
        shutdown(origin_sock, 2);
        return -7;
    }
    bytes_written += cur_bytes_written;

    // write end to end headers to file
    const char* content_length_header = "Content-Length";
    const char* content_type_header = "Content-Type";
    const char* content_encoding_header = "Content-Encoding";
    const StringView content_length_sc = {
        .data = content_length_header, .length = strlen(content_length_header)
    };
    const StringView content_type_sc = {
        .data = content_type_header, .length = strlen(content_type_header)
    };
    const StringView content_encoding_sc = {
        .data = content_encoding_header, .length = strlen(content_encoding_header)
    };
    int content_length = 0;
    for (int i = 0; i < header_count; i++) {
        StringView http_header[2] = {};
        int wc = sv_split_n(
            http_header, 2, http_headers_pass[i].data, http_headers_pass[i].length, ":", true
        );
        if (wc < 2) {
            continue;
        }

        bool is_content_length = false;
        if ((is_content_length = sv_cmp(http_header[0], content_length_sc, false)) ||
            sv_cmp(http_header[0], content_type_sc, false) ||
            sv_cmp(http_header[0], content_encoding_sc, false)) {
            cur_bytes_written =
                write(fd_to_write, http_headers_pass[i].data, http_headers_pass[i].length);
            if (cur_bytes_written != http_headers_pass[i].length) {
                shutdown(origin_sock, 2);
                return -8;
            }
            bytes_written += cur_bytes_written;

            cur_bytes_written = write(fd_to_write, "\r\n", 2);
            if (cur_bytes_written != 2) {
                shutdown(origin_sock, 2);
                return -9;
            }
            bytes_written += cur_bytes_written;
        }

        // get content length as int
        if (is_content_length) {
            sv_strip(&http_header[1]);
            static char int_buffer[256];
            memcpy(int_buffer, http_header[1].data, http_header[1].length);
            int_buffer[http_header[1].length] = '\0';
            content_length = atoi(int_buffer);
        }
    }

    // indicate to reader we are finished with headers
    cur_bytes_written = write(fd_to_write, "\r\n", 2);
    if (cur_bytes_written != 2) {
        shutdown(origin_sock, 2);
        return -10;
    }
    bytes_written += cur_bytes_written;

    //
    // write file from origin
    //
    int file_bytes_written = 0;
    // write body from first call to recv
    if (header_body_wc == 2) {
        int cur_file_bytes_written =
            write(fd_to_write, header_body_pass[1].data, header_body_pass[1].length);
        if (cur_file_bytes_written != header_body_pass[1].length) {
            // did not write all file bytes from first recv
            shutdown(origin_sock, 2);
            return -11;
        }
        file_bytes_written += cur_file_bytes_written;
    }

    while (file_bytes_written < content_length) {
        bytes_recv = recv(origin_sock, recv_buffer, PC_BUFFER_SIZE, 0);
        if (bytes_recv < 0) {
            // recv failed for rest of file
            shutdown(origin_sock, 2);
            return -12;
        }
        int cur_file_bytes_written = write(fd_to_write, recv_buffer, bytes_recv);
        if (cur_file_bytes_written != bytes_recv) {
            // did not write all bytes from rest of file
            shutdown(origin_sock, 2);
            return -13;
        }
        file_bytes_written += cur_file_bytes_written;
    }

    shutdown(origin_sock, 2);
    return 0;
}
