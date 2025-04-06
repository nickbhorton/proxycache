#include "request.h"
#include "string_view.h"
#include "url.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

int parse_request(const char* src, size_t src_length, HttpRequest* dest) {
    StringView head_pass[2];
    // making sure we remove body
    sv_split_n(head_pass, 2, src, src_length, "\r\n\r\n", true);
    StringView line_pass[2];
    int rv = sv_split_n(line_pass, 2, head_pass[0].data, head_pass[0].length, "\r\n", true);
    if (rv == 2) {
        dest->headers = line_pass[1];
        // the bad math again
        dest->headers.length = head_pass[0].length - line_pass[0].length - 2;
    }

    StringView url_pass[3];
    rv = sv_split_n(url_pass, 3, line_pass[0].data, line_pass[0].length, " ", false);
    if (rv != 3) {
        return 400;
    }
    if (url_pass[0].length != 3) {
        return 400;
    }
    if (strncmp(url_pass[0].data, "GET", url_pass[0].length) != 0) {
        return 400;
    }
    if (url_pass[2].length != 8) {
        return 400;
    }
    if (strncmp(url_pass[2].data, "HTTP/1.0", url_pass[2].length) != 0) {
        dest->version = HTTP_VERSION_10;
    } else if (strncmp(url_pass[2].data, "HTTP/1.1", url_pass[2].length) != 0) {
        dest->version = HTTP_VERSION_11;
    } else {
        return 400;
    }

    dest->url = url_pass[1];
    return 0;
}

static const char end_to_end_headers[][256] = {
    "Accept", "Accept-Language", "Accept-Encoding", "Host"
};
static const StringView end_to_end_headers_sv[] = {
    {.data = end_to_end_headers[0], .length = 6},
    {.data = end_to_end_headers[1], .length = 15},
    {.data = end_to_end_headers[2], .length = 15},
    {.data = end_to_end_headers[3], .length = 4},
};

int to_proxy_request(
    const char* client_request, size_t client_request_length, char proxy_request[HTTP_REQUEST_SIZE],
    Url* url
) {
    HttpRequest parsed_request = {};
    int rv = parse_request(client_request, client_request_length, &parsed_request);
    if (rv != 0) {
        return -rv;
    }
    rv = parse_url(parsed_request.url.data, parsed_request.url.length, url);
    if (rv < 0) {
        return -400;
    }

    char* write_ptr = proxy_request;
    memcpy(write_ptr, "GET /", 5);
    write_ptr += 5;
    memcpy(write_ptr, url->path.data, url->path.length);
    write_ptr += url->path.length;
    memcpy(write_ptr, " HTTP/1.1\r\n", 11);
    write_ptr += 11;

    static StringView sv_buffer[128];
    rv = sv_split_n(
        sv_buffer, 128, parsed_request.headers.data, parsed_request.headers.length, "\r\n", true
    );
    static StringView header_breakdown[2];
    for (int i = 0; i < rv; i++) {
        int wc = sv_split_n(header_breakdown, 2, sv_buffer[i].data, sv_buffer[i].length, ":", true);
        if (wc != 2) {
            continue;
        }
        sv_strip(&header_breakdown[0]);
        sv_strip(&header_breakdown[1]);

        // put end to end headers back
        for (size_t e = 0; e < sizeof(end_to_end_headers) / sizeof(end_to_end_headers[0]); e++) {
            if (sv_cmp(header_breakdown[0], end_to_end_headers_sv[e], false)) {
                memcpy(write_ptr, end_to_end_headers_sv[e].data, end_to_end_headers_sv[e].length);
                write_ptr += end_to_end_headers_sv[e].length;
                memcpy(write_ptr, ": ", 2);
                write_ptr += 2;
                memcpy(write_ptr, header_breakdown[1].data, header_breakdown[1].length);
                write_ptr += header_breakdown[1].length;
                memcpy(write_ptr, "\r\n", 2);
                write_ptr += 2;
            }
        }
    }

    // add proxy to origin specific headers
    static const char* user_agent_header = "User-Agent: nbh_proxy_cache\r\n";
    memcpy(write_ptr, user_agent_header, strlen(user_agent_header));
    write_ptr += strlen(user_agent_header);

    // signify headers are done
    memcpy(write_ptr, "\r\n", 2);
    write_ptr += 2;
    return write_ptr - proxy_request;
}
