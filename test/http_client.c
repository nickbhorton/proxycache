#include "http_client.h"
#include "entry.h"
#include "string_view.h"
#include "url.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
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

    FILE* fptr = fopen("test.data", "w");
    int rv = cl_get(example_http_request, &url, fileno(fptr));
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

    FILE* fptr = fopen("test.data", "w");
    int rv = cl_get(example_http_request, &url, fileno(fptr));
    CU_ASSERT(rv == 0);
    if (rv != 0) {
        printf("%d\n", rv);
    }
}

void cl_example_request_atomic() {
    StringView example_http_request = {
        .data = "GET /test.txt HTTP/1.1\r\n"
                "Host: 127.0.0.1\r\n"
                "User-Agent: nbh_pc_http_client\r\n"
                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                "Accept-Language: en-US,en;q=0.5\r\n"
                "Priority: u=0, i\r\n\r\n",
    };
    example_http_request.length = strlen(example_http_request.data);

    const Url url = {
        .domain = {.data = "127.0.0.1", .length = strlen("127.0.0.1")},
        .path = {.data = "/test.txt", .length = strlen("/test.txt")},
        .port = 8889,
        .scheme = {.data = "http", .length = strlen("http")},
        .parameters_anchor = {}
    };

    StringView filename = {.data = "test.txt", .length = strlen("test.txt")};
    remove("test.txt");
    int passed = 0;
    if (!fork()) {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            if (bytes_read == strlen("hello world") + 1) {
                if (strncmp(read_buffer, "hello world", strlen("hello world")) == 0) {
                    passed = 1;
                }
            }
            close(fd);
        }
        if (passed) {
            exit(EXIT_SUCCESS);
        }
        CU_ASSERT(0);
    } else {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        CU_ASSERT(fd > 0);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            CU_ASSERT(bytes_read == strlen("hello world") + 1);
            if (bytes_read == strlen("hello world") + 1) {
                CU_ASSERT(strncmp(read_buffer, "hello world", strlen("hello world")) == 0);
            }
            close(fd);
        }
        wait(NULL);
    }
}

void add_http_client_test() {
    CU_pSuite suite = CU_add_suite("http_client", 0, 0);
    CU_add_test(suite, "getting index.html from nbws", cl_example_request);
    CU_add_test(suite, "getting an image from nbws", cl_example_request_with_image);
    CU_add_test(suite, "getting index.html from nbws atomic", cl_example_request_atomic);
}
