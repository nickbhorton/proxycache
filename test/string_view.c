#include "string_view.h"
#include "entry.h"

#include <string.h>
#include <unistd.h>

void sv_get_words_http_line_header_1() {
    const char* to_parse = "GET http://example.com:80 HTTP/1.1\r\n";
    StringView sva[3] = {};
    int rv = sv_split_n(sva, 3, to_parse, strlen(to_parse), " ", true);
    CU_ASSERT(rv == 3);
    CU_ASSERT(sva[0].length == 3);
    CU_ASSERT(sva[1].length == 21);
    CU_ASSERT(sva[2].length == 10);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[0].data, to_parse, 3) == 0);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[1].data, to_parse + 4, 21) == 0);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[2].data, to_parse + 26, 10) == 0);
}

void sv_get_words_http_line_header_2() {
    const char* to_parse = "GET http://example.com:80 HTTP/1.1\r\n";
    StringView sva[3] = {};
    int rv = sv_split_n(sva, 3, to_parse, strlen(to_parse), " ", false);
    CU_ASSERT(rv == 3);
    CU_ASSERT(sva[0].length == 3);
    CU_ASSERT(sva[1].length == 21);
    CU_ASSERT(sva[2].length == 10);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[0].data, to_parse, 3) == 0);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[1].data, to_parse + 4, 21) == 0);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[2].data, to_parse + 26, 10) == 0);
}

void sv_get_words_http_line_header_3() {
    const char* to_parse = "GET   http://example.com:80   HTTP/1.1\r\n";
    StringView sva[3] = {};
    int rv = sv_split_n(sva, 3, to_parse, strlen(to_parse), " ", false);
    CU_ASSERT(rv == 3);
    CU_ASSERT(sva[0].length == 3);
    CU_ASSERT(sva[1].length == 21);
    CU_ASSERT(sva[2].length == 10);
    if (sva[0].data != NULL) {
        CU_ASSERT(strncmp(sva[0].data, to_parse, 3) == 0);
    } else
        CU_ASSERT(sva[0].data != NULL)
    if (sva[1].data != NULL) {
        CU_ASSERT(strncmp(sva[1].data, to_parse + 6, 21) == 0);
    } else
        CU_ASSERT(sva[1].data != NULL)
    if (sva[2].data != NULL) {
        CU_ASSERT(strncmp(sva[2].data, to_parse + 30, 10) == 0);
    } else
        CU_ASSERT(sva[2].data != NULL)
}

static const char proxy_http_req_array[10][128] = {
    "GET http://www.example.com/ HTTP/1.1\r\n",
    "Host: www.example.com\r\n",
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n",
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n",
    "Accept-Language: en-US,en;q=0.5\r\n",
    "Accept-Encoding: gzip, deflate\r\n",
    "Connection: keep-alive\r\n",
    "Upgrade-Insecure-Requests: 1\r\n",
    "Priority: u=0, i\r\n",
    "\r\n"
};

static const char* proxy_http_req =
    "GET http://www.example.com/ HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
    "Accept-Language: en-US,en;q=0.5\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Connection: keep-alive\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "Priority: u=0, i\r\n"
    "\r\n";

void sv_get_words_http_request_1() {

    StringView sva[16] = {};
    int rv = sv_split_n(sva, 16, proxy_http_req, strlen(proxy_http_req), "\r\n", true);
    CU_ASSERT(rv == 10);
    for (size_t i = 0; i < 10; i++) {
        CU_ASSERT(sva[i].length == strlen(proxy_http_req_array[i]) - 2);
    }
    for (size_t i = 0; i < 10; i++) {
        if (sva[i].data != NULL) {
            CU_ASSERT(
                strncmp(
                    sva[i].data, proxy_http_req_array[i], strlen(proxy_http_req_array[i]) - 2
                ) == 0
            );
        } else
            CU_ASSERT(sva[i].data != NULL);
    }
}

void add_string_view_tests() {
    CU_pSuite suite = CU_add_suite("string_view", 0, 0);
    CU_add_test(suite, "sv_split_n with http top line (1/3)", sv_get_words_http_line_header_1);
    CU_add_test(suite, "sv_split_n with http top line (2/3)", sv_get_words_http_line_header_2);
    CU_add_test(suite, "sv_split_n with http top line (3/3)", sv_get_words_http_line_header_3);
    CU_add_test(suite, "sv_split_n with proxy http request(1/1)", sv_get_words_http_request_1);
}
