#include "prefetch.h"
#include "connection.h"
#include "filename.h"
#include "http_client.h"
#include "string_view.h"

#include <fcntl.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

StringView extract_link(const StringView* line, const char* first_split) {
    StringView result = {.data = NULL, .length = 0};
    StringView first_pass[2];
    int wc = sv_split_n(first_pass, 2, line->data, line->length, first_split, true);
    if (wc != 2) {
        return result;
    }
    StringView second_pass[2];
    wc = sv_split_n(second_pass, 2, first_pass[1].data, first_pass[1].length, "\"", true);
    if (wc != 2) {
        return result;
    }
    result = second_pass[0];
    if (result.length >= 2 && result.data[0] == '.' && result.data[1] == '/') {
        result.data += 2;
        result.length -= 2;
        return result;
    }
    // throw away actual links
    const char* https_start = "https://";
    const char* http_start = "http://";
    if (strncmp(result.data, https_start, strlen(https_start)) == 0) {
        result.data = NULL;
        result.length = 0;
        return result;
    }
    if (strncmp(result.data, http_start, strlen(http_start)) == 0) {
        result.data = NULL;
        result.length = 0;
        return result;
    }
    if (result.length == 1 && result.data[0] == '#') {
        result.data = NULL;
        result.length = 0;
        return result;
    }
    return result;
}

int cache_new_file(StringView link, StringView old_proxy_request, const Url* url) {
    if (old_proxy_request.length > PC_BUFFER_SIZE) {
        return -1;
    }
    char new_proxy_request[PC_BUFFER_SIZE] = {};
    StringView first_pass[2];
    int wc = sv_split_n(first_pass, 2, old_proxy_request.data, old_proxy_request.length, "/", true);
    if (wc != 2) {
        return -1;
    }
    size_t request_size = 0;
    char* buffer_ptr = memcpy(new_proxy_request, first_pass[0].data, first_pass[0].length);
    buffer_ptr += first_pass[0].length;
    request_size += first_pass[0].length;
    memcpy(buffer_ptr, "/", 1);
    buffer_ptr++;
    request_size++;
    memcpy(buffer_ptr, link.data, link.length);
    buffer_ptr += link.length;
    request_size += link.length;
    memcpy(
        buffer_ptr, first_pass[1].data,
        ((int)old_proxy_request.length) - ((int)first_pass[0].length) - 1
    );
    buffer_ptr += ((int)old_proxy_request.length) - ((int)first_pass[0].length) - 1;
    request_size += ((int)old_proxy_request.length) - ((int)first_pass[0].length) - 1;
    Url new_url = *url;
    new_url.path = link;
    printf("prefetching %.*s\n", (int)new_url.path.length, new_url.path.data);

    char filename[39];
    get_filename(&new_url, filename);
    StringView filename_sv = {.data = filename, .length = 38};
    StringView new_proxy_request_sv = {.data = new_proxy_request, .length = request_size};
    int fd = cl_get_atomic(new_proxy_request_sv, &new_url, filename_sv);
    if (fd < 0) {
        return fd;
    }
    close(fd);
    return 0;
}

int prefetch(const char* filename, StringView proxy_request, const Url* url, size_t file_length) {
    // read file
    char* file_bytes = malloc(file_length);
    int requested_file_fd = open(filename, O_RDONLY);
    if (requested_file_fd < 0) {
        return -1;
    }
    int bytes_read = read(requested_file_fd, file_bytes, file_length);
    if (bytes_read < file_length) {
        return -1;
    }
    close(requested_file_fd);

    // count lines
    unsigned int line_count = 0;
    for (size_t i = 0; i < file_length; i++) {
        if (file_bytes[i] == '\n') {
            line_count++;
        }
    }

    // malloc svs
    StringView* lines = malloc(sizeof(StringView) * line_count);
    int wc = sv_split_n(lines, line_count, file_bytes, file_length, "\n", true);
    // forking this again causes less performance
    for (int i = 0; i < wc; i++) {
        char line_buffer[4096];
        if (lines[i].length > 4095) {
            continue;
        }
        memcpy(line_buffer, lines[i].data, lines[i].length);
        line_buffer[lines[i].length] = '\0';
        if (fnmatch("*src=\"*", line_buffer, 0) == 0) {
            StringView link = extract_link(&lines[i], "src=\"");
            if (link.length > 0) {
                cache_new_file(link, proxy_request, url);
            }
        }
        if (fnmatch("*href=\"*", line_buffer, 0) == 0) {
            StringView link = extract_link(&lines[i], "href=\"");
            if (link.length > 0) {
                cache_new_file(link, proxy_request, url);
            }
        }
    }

    free(lines);
    free(file_bytes);
    return 0;
}
