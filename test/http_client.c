#include "http_client.h"
#include "entry.h"
#include "string_view.h"
#include "url.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

void cl_example_request() {
    StringView example_http_request = {
        .data = "GET / HTTP/1.1\r\n"
                "Host: 127.0.0.1\r\n"
                "User-Agent: nbh_pc_http_client\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: en-US,en;q=0.5\r\n"
                "Priority: u=0, i\r\n\r\n",
    };
    example_http_request.length = strlen(example_http_request.data);

    const Url url = {
        .domain = {.data = "127.0.0.1", .length = strlen("127.0.0.1")},
        .path = {.data = "/", .length = strlen("/")},
        .port = 8889,
        .scheme = {.data = "http", .length = strlen("http")},
        .parameters_anchor = {}
    };

    StringView filename = {.data = "test.data", .length = strlen("test.data")};

    int rv = cl_get(example_http_request, &url, filename);
    CU_ASSERT(rv == 0);
    if (rv != 0) {
        printf("%d\n", rv);
    }
}

void cl_example_request_with_image() {
    StringView example_http_request = {
        .data = "GET /img/img_2026.jpg HTTP/1.1\r\n"
                "Host: 127.0.0.1\r\n"
                "User-Agent: nbh_pc_http_client\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: en-US,en;q=0.5\r\n"
                "Priority: u=0, i\r\n\r\n",
    };
    example_http_request.length = strlen(example_http_request.data);

    const Url url = {
        .domain = {.data = "127.0.0.1", .length = strlen("127.0.0.1")},
        .path = {.data = "/", .length = strlen("/")},
        .port = 8889,
        .scheme = {.data = "http", .length = strlen("http")},
        .parameters_anchor = {}
    };

    StringView filename = {.data = "test.data", .length = strlen("test.data")};

    int rv = cl_get(example_http_request, &url, filename);
    CU_ASSERT(rv == 0);
    if (rv != 0) {
        printf("%d\n", rv);
    }
}

void add_http_client_test() {
    CU_pSuite suite = CU_add_suite("http_client", 0, 0);
    CU_add_test(suite, "getting index.html from nbws", cl_example_request);
    CU_add_test(suite, "getting an image from nbws", cl_example_request_with_image);
}
