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

void parse_url_mdn_2() {
    // no port specified
    const char* url_cstr = "http"
                           "://"
                           "www.example.com"
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

void parse_url_mdn_3() {
    // no port specified and no parameters and anchor
    const char* url_cstr = "http"
                           "://"
                           "www.example.com/path/to/myfile.html";
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

    CU_ASSERT(url.parameters_anchor.length == 0);
}

void parse_url_mdn_4() {
    // no port specified and no parameters and anchor and path is /
    const char* url_cstr = "http"
                           "://"
                           "www.example.com/";
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

    CU_ASSERT(url.path.length == 0);
    CU_ASSERT(url.parameters_anchor.length == 0);
}

void parse_url_mdn_5() {
    // no port specified and no parameters and anchor and no path
    const char* url_cstr = "http://www.example.com";
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

    CU_ASSERT(url.path.length == 0);
    CU_ASSERT(url.parameters_anchor.length == 0);
}

void parse_url_mdn_6() {
    // checking for easy failures
    const char* url_cstr_f1 = "www.example.com";
    const char* url_cstr_f2 = "http:/www.example.com";
    Url url = {};
    int rv = parse_url(url_cstr_f1, strlen(url_cstr_f1), &url);
    CU_ASSERT(rv == -1);
    rv = parse_url(url_cstr_f2, strlen(url_cstr_f2), &url);
    CU_ASSERT(rv == -1);
}

void parse_url_mdn_7() {
    // different port
    const char* url_cstr = "http://www.example.com:8888/";
    Url url = {};
    int rv = parse_url(url_cstr, strlen(url_cstr), &url);
    CU_ASSERT_FATAL(rv == 0);
    CU_ASSERT(url.port == 8888);

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

    CU_ASSERT(url.path.length == 0);
    CU_ASSERT(url.parameters_anchor.length == 0);
}

void add_url_tests() {
    CU_pSuite suite = CU_add_suite("url", 0, 0);
    CU_add_test(suite, "parse_rul with mdn example url (1/7)", parse_url_mdn_1);
    CU_add_test(suite, "parse_rul with mdn example url (2/7)", parse_url_mdn_2);
    CU_add_test(suite, "parse_rul with mdn example url (3/7)", parse_url_mdn_3);
    CU_add_test(suite, "parse_rul with mdn example url (4/7)", parse_url_mdn_4);
    CU_add_test(suite, "parse_rul with mdn example url (5/7)", parse_url_mdn_5);
    CU_add_test(suite, "parse_rul with mdn example url (6/7)", parse_url_mdn_6);
    CU_add_test(suite, "parse_rul with mdn example url (7/7)", parse_url_mdn_7);
}
