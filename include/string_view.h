#ifndef PROXY_CACHE_STRING_VIEW_HEADER
#define PROXY_CACHE_STRING_VIEW_HEADER

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char* data;
    size_t length;
} StringView;

int sv_split_n(
    StringView* dest, size_t dest_count, const char* src, size_t src_length, const char* split,
    bool exact_amount
);

#endif
