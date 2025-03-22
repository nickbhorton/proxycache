#include "http.h"
#include "string_view.h"

#include <stddef.h>
#include <string.h>

int parse_request(const char* src, size_t src_length, HttpRequest* dest) {
    StringView head_pass[2];
    // making sure we remove body
    sv_split_n(head_pass, 2, src, src_length, "\r\n\r\n", true);
    StringView line_pass[2];
    int rv = sv_split_n(line_pass, 2, head_pass[0].data, head_pass[0].length, "\r\n", true);
    if (rv != 2) {
        return 400;
    }
    dest->headers = line_pass[1];
    // the bad math again
    dest->headers.length = head_pass[0].length - line_pass[0].length - 2;
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
