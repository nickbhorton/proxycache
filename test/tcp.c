#include "tcp.h"
#include "entry.h"

#include <unistd.h>

void happy_http_connect() {
    const char* domain_cstr = "www.example.com";
    StringView domain = {.data = domain_cstr, .length = strlen(domain_cstr)};
    int socket_fd = tcp_connect(&domain, 80, false);
    CU_ASSERT(socket_fd >= 0);
    close(socket_fd);

    domain_cstr = "socialstudieshelp.com";
    domain.data = domain_cstr;
    domain.length = strlen(domain_cstr);
    socket_fd = tcp_connect(&domain, 80, false);
    CU_ASSERT(socket_fd >= 0);
    close(socket_fd);
}

void add_utils_tests() {
    CU_pSuite suite = CU_add_suite("utils", 0, 0);
    CU_add_test(suite, "http_connect to known http server", happy_http_connect);
}
