#ifndef PROXY_CACHE_FILENAME_HEADER
#define PROXY_CACHE_FILENAME_HEADER

#include <stdio.h>

#include "url.h"

void get_filename(const Url* url, char filename_o[39]);
ssize_t get_filesize(const char* filename);

#endif
