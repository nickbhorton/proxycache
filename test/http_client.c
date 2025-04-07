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
    const char* correct_response = "HTTP/1.1 200 Ok\r\nContent-Type: text/plain\r\nContent-Length: "
                                   "12\r\nConnection: close\r\n\r\nhello "
                                   "world\n";

    int child_passed = 0;
    if (!fork()) {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            // printf("child read:\n%.*s", bytes_read, read_buffer);
            if (bytes_read == strlen(correct_response)) {
                if (strncmp(read_buffer, correct_response, strlen(correct_response)) == 0) {
                    child_passed = 1;
                }
            }
            close(fd);
            if (child_passed != 1) {
                printf("child bytes read %d\n", bytes_read);
            }
        }
        if (child_passed) {
            exit(EXIT_SUCCESS);
        }
        if (fd < 0) {
            printf("child %d\n", fd);
        }
        CU_ASSERT(0);
    } else {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        CU_ASSERT(fd > 0);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            // printf("parent read:\n%.*s", bytes_read, read_buffer);
            CU_ASSERT(bytes_read == strlen(correct_response));
            if (bytes_read == strlen(correct_response)) {
                CU_ASSERT(strncmp(read_buffer, correct_response, strlen(correct_response)) == 0);
            }
            close(fd);
        }
        if (fd < 0) {
            printf("parent %d\n", fd);
        }
        wait(NULL);
    }
}

void cl_404_from_server() {
    StringView example_http_request = {
        .data = "GET /test.xt HTTP/1.1\r\n"
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
    int child_passed = 0;
    const char* correct_response = "HTTP/1.1 404 Not Found\r\n\r\n";
    if (!fork()) {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            printf("child read:\n%.*s", bytes_read, read_buffer);
            if (bytes_read == strlen(correct_response)) {
                if (strncmp(read_buffer, correct_response, strlen(correct_response)) == 0) {
                    child_passed = 1;
                }
            }
            close(fd);
            if (child_passed != 1) {
                printf("child bytes read %d\n", bytes_read);
            }
        }
        if (child_passed) {
            exit(EXIT_SUCCESS);
        }
        if (fd < 0) {
            printf("child %d\n", fd);
        }
        CU_ASSERT(0);
    } else {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        CU_ASSERT(fd > 0);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            // printf("parent read:\n%.*s", bytes_read, read_buffer);
            CU_ASSERT(bytes_read == strlen(correct_response));
            if (bytes_read == strlen(correct_response)) {
                CU_ASSERT(strncmp(read_buffer, correct_response, strlen(correct_response)) == 0);
            }
            close(fd);
        }
        if (fd < 0) {
            printf("parent %d\n", fd);
        }
        wait(NULL);
    }
}

void cl_no_server_exists() {
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
        .port = 8890, // wrong port
        .scheme = {.data = "http", .length = strlen("http")},
        .parameters_anchor = {}
    };

    StringView filename = {.data = "test.txt", .length = strlen("test.txt")};
    remove("test.txt");
    int child_passed = 0;
    const char* correct_response = "404 Not Found\r\n\r\n";
    if (!fork()) {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            // printf("child read:\n%.*s", bytes_read, read_buffer);
            if (bytes_read == strlen(correct_response)) {
                if (strncmp(read_buffer, correct_response, strlen(correct_response)) == 0) {
                    child_passed = 1;
                }
            }
            close(fd);
            if (child_passed != 1) {
                printf("child bytes read %d\n", bytes_read);
            }
        }
        if (child_passed) {
            exit(EXIT_SUCCESS);
        }
        if (fd < 0) {
            printf("child %d\n", fd);
        }
        CU_ASSERT(0);
    } else {
        int fd = cl_get_atomic(example_http_request, &url, filename);
        CU_ASSERT(fd > 0);
        if (fd > 0) {
            char read_buffer[256];
            int bytes_read = read(fd, read_buffer, 256);
            // printf("parent read:\n%.*s", bytes_read, read_buffer);
            CU_ASSERT(bytes_read == strlen(correct_response));
            if (bytes_read == strlen(correct_response)) {
                CU_ASSERT(strncmp(read_buffer, correct_response, strlen(correct_response)) == 0);
            }
            close(fd);
        }
        if (fd < 0) {
            printf("parent %d\n", fd);
        }
        wait(NULL);
    }
}

void add_http_client_test() {
    CU_pSuite suite = CU_add_suite("http_client", 0, 0);
    CU_add_test(suite, "getting index.html from nbws", cl_example_request);
    CU_add_test(suite, "getting an image from nbws", cl_example_request_with_image);
    CU_add_test(suite, "getting test.txt from nbws atomic", cl_example_request_atomic);
    CU_add_test(suite, "404 from from nbws atomic", cl_404_from_server);
    CU_add_test(suite, "404 because server does not exist", cl_no_server_exists);
}
