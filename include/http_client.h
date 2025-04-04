#ifndef PROXY_CACHE_HTTP_CLIENT_HEADER
#define PROXY_CACHE_HTTP_CLIENT_HEADER

#include "string_view.h"
#include "url.h"

// request a file from origin server and write it to fd_to_write.
// returns 0 on success, or a negitive value fail.
int cl_get(const StringView request, const Url* url, int fd_to_write);

// request a file from origin server and saves it to a file called 'filename'.
// returns a read only file descriptor to said file, or a negitive value on fail.
// if two processes request the same file at the same time this function guarentees only one request
// is made to origin.
int cl_get_atomic(const StringView request, const Url* url, const StringView filename);

#endif
