#ifndef PROXY_CACHE_UTILS_HEADER
#define PROXY_CACHE_UTILS_HEADER

#include <stdbool.h>
#include <stdint.h>

#include "string_view.h"

int tcp_connect(const StringView* node, int16_t port, bool server);

#endif
