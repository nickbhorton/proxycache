#ifndef PROXY_CACHE_HTTP_HEADER
#define PROXY_CACHE_HTTP_HEADER

#include <stddef.h>

#include "string_view.h"
#include "url.h"

// still only support versions 1.0/1.1
#define HTTP_VERSION_10 1
#define HTTP_VERSION_11 2

#define HTTP_REQUEST_SIZE 2048

// http method has to be GET
typedef struct {
    StringView url;
    StringView headers;
    int version;
} HttpRequest;

int parse_request(const char* src, size_t src_length, HttpRequest* dest);
int to_proxy_request(
    const char* client_request, size_t client_request_length, char proxy_request[HTTP_REQUEST_SIZE],
    Url* url
);

#endif
