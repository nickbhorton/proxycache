#ifndef PROXY_CACHE_URL_HEADER
#define PROXY_CACHE_URL_HEADER

#include <stdint.h>

#include "string_view.h"

// https://developer.mozilla.org/en-US/docs/Learn_web_development/Howto/Web_mechanics/What_is_a_URL
typedef struct {
    StringView scheme;
    StringView domain;
    int16_t port;
    StringView path;
    StringView parameters_anchor;
} Url;

int parse_url(const char* url, size_t url_length, Url* dest);

#endif
