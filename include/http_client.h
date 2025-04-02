#ifndef PROXY_CACHE_HTTP_CLIENT_HEADER
#define PROXY_CACHE_HTTP_CLIENT_HEADER

#include "string_view.h"
#include "url.h"

int cl_get(const StringView request, const Url* url, const StringView filename_to_write);

#endif
