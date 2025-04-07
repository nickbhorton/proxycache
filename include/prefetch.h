#ifndef PROXY_CACHE_PREFETCH_PARSING_HEADER
#define PROXY_CACHE_PREFETCH_PARSING_HEADER

#include "string_view.h"
#include "url.h"

// this should run in a new process on a file that was just downloaded if it is html
int prefetch(const char* filename, StringView proxy_request, const Url* url, size_t file_length);

#endif
