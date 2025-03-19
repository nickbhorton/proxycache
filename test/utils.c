#include "utils.h"
#include "entry.h"

#include <unistd.h>

void happy_http_connect() {
    int socket_fd = http_connect("www.example.com");
    CU_ASSERT(socket_fd >= 0);
    close(socket_fd);

    socket_fd = http_connect("socialstudieshelp.com");
    CU_ASSERT(socket_fd >= 0);
    close(socket_fd);
}

void happy_http_server_exists() {
    CU_ASSERT_TRUE(http_server_exists("www.example.com"));
    CU_ASSERT_TRUE(http_server_exists("socialstudieshelp.com"));
    // CU_ASSERT_FALSE(http_server_exists("jfdkkjfjfa32451jfsajfsj1432143"));
}

void add_utils_tests() {
    CU_pSuite suite = CU_add_suite("utils", 0, 0);
    CU_add_test(suite, "http_connect to known http server", happy_http_connect);
    CU_add_test(suite, "http_server_exists to assortment of urls", happy_http_server_exists);
}
