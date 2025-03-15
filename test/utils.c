#include "utils.h"
#include "entry.h"

void happy_http_connect() {
    int socket_fd = http_connect("www.example.com");
    CU_ASSERT(socket_fd >= 0);
}

void add_utils_tests() {
    CU_pSuite suite = CU_add_suite("utils", 0, 0);
    CU_add_test(suite, "happy case for http_connect with www.example.com", happy_http_connect);
}
