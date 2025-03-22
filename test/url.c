#include "url.h"
#include "entry.h"

void parse_url_mdn_1() {
    const char* url_cstr = "http"
                           "://"
                           "www.example.com:80"
                           "/path/to/myfile.html"
                           "?key1=value1&key2=value2#SomewhereInTheDocument";
    Url url = {};
    int rv = parse_url(url_cstr, strlen(url_cstr), &url);
    CU_ASSERT_FATAL(rv == 0);
    CU_ASSERT(url.port == 80);

    const char* scheme = "http";
    CU_ASSERT(url.scheme.length == strlen(scheme));
    if (url.scheme.data) {
        CU_ASSERT(strncmp(url.scheme.data, scheme, url.scheme.length) == 0);
    } else
        CU_ASSERT(1 == 0);

    const char* domain = "www.example.com";
    CU_ASSERT(url.domain.length == strlen(domain));
    if (url.domain.data) {
        CU_ASSERT(strncmp(url.domain.data, domain, url.domain.length) == 0);
    } else
        CU_ASSERT(1 == 0);

    const char* path = "path/to/myfile.html";
    CU_ASSERT(url.path.length == strlen(path));
    if (url.path.data) {
        CU_ASSERT(strncmp(url.path.data, path, url.path.length) == 0);
    } else
        CU_ASSERT(1 == 0);

    const char* parameters_anchor = "key1=value1&key2=value2#SomewhereInTheDocument";
    CU_ASSERT(url.parameters_anchor.length == strlen(parameters_anchor));
    if (url.parameters_anchor.data) {
        CU_ASSERT(
            strncmp(url.parameters_anchor.data, parameters_anchor, url.parameters_anchor.length) ==
            0
        );
    } else
        CU_ASSERT(1 == 0);
}

void add_url_tests() {
    CU_pSuite suite = CU_add_suite("url", 0, 0);
    CU_add_test(suite, "parse_rul with mdn example url (1/1)", parse_url_mdn_1);
}
