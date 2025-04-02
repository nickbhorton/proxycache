#include "entry.h"

int main() {
    CU_initialize_registry();
    add_utils_tests();
    add_string_view_tests();
    add_url_tests();
    add_http_tests();
    add_http_client_test();
    CU_basic_run_tests();
    CU_cleanup_registry();
}
