#ifndef PROXY_CACHE_UTILS_HEADER
#define PROXY_CACHE_UTILS_HEADER

#include <stdbool.h>

int http_connect(const char* node);
bool http_server_exists(const char* node);

#endif
