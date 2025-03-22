#include "http.h"

#include "entry.h"

void parse_http_firefox_1() {
    // checking for easy failures

    const char* req_cstr =
        "GET http://www.example.com/ HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: keep-alive\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Priority: u=0, i\r\n"
        "\r\n"
        "<html>this is a request body!?</html>";
    const char* headers =
        "Host: www.example.com\r\n"
        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:136.0) Gecko/20100101 Firefox/136.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Connection: keep-alive\r\n"
        "Upgrade-Insecure-Requests: 1\r\n"
        "Priority: u=0, i";

    HttpRequest req = {};
    int rv = parse_request(req_cstr, strlen(req_cstr), &req);
    CU_ASSERT_FATAL(rv == 0);
    CU_ASSERT(req.url.length == strlen("http://www.example.com/"));
    if (req.url.data) {
        CU_ASSERT(strncmp(req.url.data, "http://www.example.com/", req.url.length) == 0);
    } else
        CU_ASSERT(1 == 0);

    CU_ASSERT(req.headers.length == strlen(headers));
    if (req.headers.data) {
        CU_ASSERT(strncmp(req.headers.data, headers, req.headers.length) == 0);
    } else
        CU_ASSERT(1 == 0);
}

void add_http_tests() {
    CU_pSuite suite = CU_add_suite("http", 0, 0);
    CU_add_test(suite, "parse_request with firefox proxy example (1/1)", parse_http_firefox_1);
}
