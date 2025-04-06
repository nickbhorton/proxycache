#include "http.h"

#include "entry.h"

void parse_http_firefox_1() {
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

void to_proxy_request_http_firefox_1() {
    const char* client_request =
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
    const char* proxy_request =
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Accept-Language: en-US,en;q=0.5\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "User-Agent: nbh_proxy_cache\r\n"
        "\r\n";

    char proxy_request_buffer[HTTP_REQUEST_SIZE] = {};
    Url url = {};
    int rv = to_proxy_request(client_request, strlen(client_request), proxy_request_buffer, &url);
    CU_ASSERT_FATAL(rv > 0);
    CU_ASSERT(strncmp(proxy_request, proxy_request_buffer, strlen(proxy_request)) == 0);

    // validating the returned url
    CU_ASSERT(url.port == 80);
    CU_ASSERT(url.path.length == 0);
    CU_ASSERT(url.parameters_anchor.length == 0);

    const char* scheme = "http";
    CU_ASSERT(url.scheme.length == strlen(scheme));
    if (url.scheme.data) {
        CU_ASSERT(strncmp(url.scheme.data, scheme, strlen(scheme)) == 0);
    }
    const char* domain = "www.example.com";
    CU_ASSERT(url.domain.length == strlen(domain));
    if (url.scheme.data) {
        CU_ASSERT(strncmp(url.domain.data, domain, strlen(domain)) == 0);
    }
}

void to_proxy_request_http_firefox_2() {
    const char* client_request = "GET http://www.example.com:25565/ HTTP/1.1\r\n"
                                 "\r\n";
    const char* proxy_request = "GET / HTTP/1.1\r\n"
                                "User-Agent: nbh_proxy_cache\r\n"
                                "\r\n";

    char proxy_request_buffer[HTTP_REQUEST_SIZE] = {};
    Url url = {};
    int rv = to_proxy_request(client_request, strlen(client_request), proxy_request_buffer, &url);
    CU_ASSERT_FATAL(rv > 0);
    CU_ASSERT(strncmp(proxy_request, proxy_request_buffer, strlen(client_request)) == 0);

    // validating the returned url
    CU_ASSERT(url.port == 25565);
    CU_ASSERT(url.path.length == 0);
    CU_ASSERT(url.parameters_anchor.length == 0);

    const char* scheme = "http";
    CU_ASSERT(url.scheme.length == strlen(scheme));
    if (url.scheme.data) {
        CU_ASSERT(strncmp(url.scheme.data, scheme, strlen(scheme)) == 0);
    }
    const char* domain = "www.example.com";
    CU_ASSERT(url.domain.length == strlen(domain));
    if (url.scheme.data) {
        CU_ASSERT(strncmp(url.domain.data, domain, strlen(domain)) == 0);
    }
}

void add_http_tests() {
    CU_pSuite suite = CU_add_suite("http", 0, 0);
    CU_add_test(suite, "parse_request with firefox proxy example (1/1)", parse_http_firefox_1);
    CU_add_test(
        suite, "to_proxy_request with firefox proxy example (1/x)", to_proxy_request_http_firefox_1
    );
    CU_add_test(
        suite, "to_proxy_request with firefox proxy example (2/x)", to_proxy_request_http_firefox_2
    );
}
