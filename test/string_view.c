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
    if (sva[1].data != NULL)
        CU_ASSERT(strncmp(sva[1].data, to_parse + 4, 21) == 0);
    if (sva[2].data != NULL)
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
    if (sva[1].data != NULL)
        CU_ASSERT(strncmp(sva[1].data, to_parse + 4, 21) == 0);
    if (sva[2].data != NULL)
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

void sv_get_words_http_line_header_4() {
    // only getting the first word
    const char* to_parse = "GET http://example.com:80 HTTP/1.1\r\n";
    StringView sva[1] = {};
    int rv = sv_split_n(sva, 1, to_parse, strlen(to_parse), " ", true);
    CU_ASSERT(rv == 1);
    CU_ASSERT(sva[0].length == 3);
    if (sva[0].data != NULL)
        CU_ASSERT(strncmp(sva[0].data, to_parse, 3) == 0);
}

void sv_get_words_http_line_header_5() {
    // getting the first two words
    const char* to_parse = "GET   http://example.com:80   HTTP/1.1\r\n";
    StringView sva[2] = {};
    int rv = sv_split_n(sva, 2, to_parse, strlen(to_parse), " ", false);
    CU_ASSERT(rv == 2);
    CU_ASSERT(sva[0].length == 3);
    CU_ASSERT(sva[1].length == 21);
    if (sva[0].data != NULL) {
        CU_ASSERT(strncmp(sva[0].data, to_parse, 3) == 0);
    } else
        CU_ASSERT(sva[0].data != NULL)
    if (sva[1].data != NULL) {
        CU_ASSERT(strncmp(sva[1].data, to_parse + 6, 21) == 0);
    } else
        CU_ASSERT(sva[1].data != NULL)
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

void sv_get_words_http_request_2() {
    // this is what an actual http workflow will look like
    StringView line = {};
    int rv = sv_split_n(&line, 1, proxy_http_req, strlen(proxy_http_req), "\r\n", true);
    CU_ASSERT(rv == 1);
    CU_ASSERT(line.length == strlen(proxy_http_req_array[0]) - 2);
    if (line.data != NULL) {
        CU_ASSERT(
            strncmp(line.data, proxy_http_req_array[0], strlen(proxy_http_req_array[0]) - 2) == 0
        );
    } else
        CU_ASSERT(line.data != NULL);
    StringView http_line_words[3];
    rv = sv_split_n(http_line_words, 3, line.data, line.length, " ", false);
    CU_ASSERT(rv == 3);
    CU_ASSERT(http_line_words[0].length == 3);
    CU_ASSERT(strncmp(http_line_words[0].data, "GET", 3) == 0);
    CU_ASSERT(http_line_words[1].length == 23);
    CU_ASSERT(strncmp(http_line_words[1].data, "http://www.example.com/", 23) == 0);
    CU_ASSERT(http_line_words[2].length == 8);
    CU_ASSERT(strncmp(http_line_words[2].data, "HTTP/1.1", 8) == 0);
}

void sv_get_words_http_request_3() {
    // like 2 with extra spaces
    const char* to_parse = "GET   http://www.example.com/     HTTP/1.1\r\n"
                           "Host: www.example.com\r\n";
    StringView line = {};
    int rv = sv_split_n(&line, 1, to_parse, strlen(proxy_http_req), "\r\n", true);
    CU_ASSERT(rv == 1);
    CU_ASSERT(line.length == 42);
    if (line.data != NULL) {
        CU_ASSERT(strncmp(line.data, to_parse, line.length) == 0);
    } else
        CU_ASSERT(line.data != NULL);
    StringView http_line_words[3];
    rv = sv_split_n(http_line_words, 3, line.data, line.length, " ", false);
    CU_ASSERT(rv == 3);
    CU_ASSERT(http_line_words[0].length == 3);
    CU_ASSERT(strncmp(http_line_words[0].data, "GET", 3) == 0);
    CU_ASSERT(http_line_words[1].length == 23);
    CU_ASSERT(strncmp(http_line_words[1].data, "http://www.example.com/", 23) == 0);
    CU_ASSERT(http_line_words[2].length == 8);
    CU_ASSERT(strncmp(http_line_words[2].data, "HTTP/1.1", 8) == 0);

    StringView url_pass1[2];
    rv = sv_split_n(url_pass1, 2, http_line_words[1].data, http_line_words[1].length, "://", true);
    CU_ASSERT(rv == 2)
    CU_ASSERT(url_pass1[0].length == 4);
    CU_ASSERT(strncmp(url_pass1[0].data, "http", url_pass1[0].length) == 0);
    CU_ASSERT(url_pass1[1].length == 16);
    CU_ASSERT(strncmp(url_pass1[1].data, "www.example.com/", url_pass1[1].length) == 0);
}

void sv_split_n_url_1() {
    const char* url = "www.example.com/";
    StringView url_pass[2];
    int rv = sv_split_n(url_pass, 2, url, strlen(url), "/", true);
    CU_ASSERT(rv == 1)
    CU_ASSERT(url_pass[0].length == 15);
    CU_ASSERT(strncmp(url_pass[0].data, "www.example.com", url_pass[0].length) == 0);
}

void sv_split_n_url_2() {
    const char* url = "www.example.com";
    StringView url_pass[2];
    int rv = sv_split_n(url_pass, 2, url, strlen(url), "/", true);
    CU_ASSERT(rv == 1)
    CU_ASSERT(url_pass[0].length == 15);
    CU_ASSERT(strncmp(url_pass[0].data, "www.example.com", url_pass[0].length) == 0);
}

void sv_split_n_url_3() {
    const char* url = "www.example.com/index.html";
    StringView url_pass[2];
    int rv = sv_split_n(url_pass, 2, url, strlen(url), "/", true);
    CU_ASSERT(rv == 2)
    CU_ASSERT(url_pass[0].length == 15);
    CU_ASSERT(strncmp(url_pass[0].data, "www.example.com", url_pass[0].length) == 0);
    CU_ASSERT(url_pass[1].length == 10);
    CU_ASSERT(strncmp(url_pass[1].data, "index.html", url_pass[1].length) == 0);
}

void sv_split_n_url_4() {
    const char* url = "www.example.com:8888/index.html";
    StringView url_pass1[2];
    int rv = sv_split_n(url_pass1, 2, url, strlen(url), "/", true);
    CU_ASSERT(rv == 2)
    CU_ASSERT(url_pass1[0].length == 20);
    CU_ASSERT(strncmp(url_pass1[0].data, "www.example.com:8888", url_pass1[0].length) == 0);
    CU_ASSERT(url_pass1[1].length == 10);
    CU_ASSERT(strncmp(url_pass1[1].data, "index.html", url_pass1[1].length) == 0);

    StringView url_pass2[2];
    rv = sv_split_n(url_pass2, 2, url_pass1[0].data, url_pass1[0].length, ":", true);
    CU_ASSERT(rv == 2)
    CU_ASSERT(url_pass2[0].length == 15);
    CU_ASSERT(strncmp(url_pass2[0].data, "www.example.com", url_pass2[0].length) == 0);
    CU_ASSERT(url_pass2[1].length == 4);
    CU_ASSERT(strncmp(url_pass2[1].data, "8888", url_pass2[1].length) == 0);
}

void sv_split_n_url_5() {
    // this is an edge case I worry about
    // The behavior here I want is if we match exact case I want to know how many spaces we have
    // for www.example.com/// -> ["www.example.com", "", ""]
    const char* url = "www.example.com///";
    StringView url_pass[4];
    int rv = sv_split_n(url_pass, 4, url, strlen(url), "/", true);
    CU_ASSERT(rv == 3);
    CU_ASSERT(url_pass[0].length == 15);
    CU_ASSERT(strncmp(url_pass[0].data, "www.example.com", url_pass[0].length) == 0);

    // for not matching amount this should act line www.example.com/
    rv = sv_split_n(url_pass, 4, url, strlen(url), "/", false);
    CU_ASSERT(rv == 1);
    CU_ASSERT(url_pass[0].length == 15);
    CU_ASSERT(strncmp(url_pass[0].data, "www.example.com", url_pass[0].length) == 0);
}

void sv_strip_tests() {
    const char examples[][2][128] = {
        {"   hello \r\n", "hello"},
        {"  \r\r\r\n want to strip\n this  \r\n", "want to strip\n this"},
        {"", ""},
        {" ", ""},
        {" t\n", "t"}
    };
    for (size_t i = 0; i < sizeof(examples) / sizeof(char[2][128]); i++) {
        StringView example_q = {.data = examples[i][0], .length = strlen(examples[i][0])};
        sv_strip(&example_q);
        StringView example_a = {.data = examples[i][1], .length = strlen(examples[i][1])};
        CU_ASSERT(example_q.length == example_a.length);
        CU_ASSERT(strncmp(example_a.data, example_q.data, example_a.length) == 0);
    }
}

void add_string_view_tests() {
    CU_pSuite suite = CU_add_suite("string_view", 0, 0);
    // sv_split_n
    CU_add_test(suite, "sv_split_n with http top line (1/5)", sv_get_words_http_line_header_1);
    CU_add_test(suite, "sv_split_n with http top line (2/5)", sv_get_words_http_line_header_2);
    CU_add_test(suite, "sv_split_n with http top line (3/5)", sv_get_words_http_line_header_3);
    CU_add_test(suite, "sv_split_n with http top line (4/5)", sv_get_words_http_line_header_4);
    CU_add_test(suite, "sv_split_n with http top line (5/5)", sv_get_words_http_line_header_5);
    CU_add_test(suite, "sv_split_n with proxy http request (1/3)", sv_get_words_http_request_1);
    CU_add_test(suite, "sv_split_n with proxy http request (2/3)", sv_get_words_http_request_2);
    CU_add_test(suite, "sv_split_n with proxy http request (3/3)", sv_get_words_http_request_3);
    CU_add_test(suite, "sv_split_n with url (1/5)", sv_split_n_url_1);
    CU_add_test(suite, "sv_split_n with url (2/5)", sv_split_n_url_2);
    CU_add_test(suite, "sv_split_n with url (3/5)", sv_split_n_url_3);
    CU_add_test(suite, "sv_split_n with url (4/5)", sv_split_n_url_4);
    CU_add_test(suite, "sv_split_n with url (5/5)", sv_split_n_url_5);

    // sv_strip
    CU_add_test(suite, "sv_strip basic examples", sv_strip_tests);
}
