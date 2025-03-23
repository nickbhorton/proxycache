#include "string_view.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

int sv_split_n(
    StringView* dest, size_t dest_count, const char* src, size_t src_length, const char* split,
    bool exact_amount
) {
    size_t split_length = strlen(split);
    if (src_length < split_length) {
        return 0;
    }
    const char* src_ptr = src;
    int sv_i = 0;
    for (; sv_i < dest_count; sv_i++) {
        dest[sv_i].data = src_ptr;
        dest[sv_i].length = 0;

        int match_count = 0;
        size_t sv_length = 0;
        // find first match
        while (!match_count) {
            // if we are at the end of src, exit
            if (src_ptr + split_length >= src + src_length) {
                if (strncmp(src_ptr, split, split_length) != 0) {
                    dest[sv_i].length = sv_length + (src + src_length - src_ptr);
                } else {
                    dest[sv_i].length = sv_length;
                }
                return sv_i + 1;
            }
            if (strncmp(src_ptr, split, split_length) == 0) {
                match_count++;
                src_ptr += split_length;
            } else {
                src_ptr++;
                sv_length++;
            }
        }
        if (!exact_amount) {
            while (strncmp(src_ptr, split, split_length) == 0) {
                src_ptr += split_length;
                // if we are at the end of src, exit
                if (src_ptr >= src + src_length) {
                    dest[sv_i].length = sv_length;
                    return sv_i + 1;
                }
            }
        }
        dest[sv_i].length = sv_length;
    }
    return sv_i;
}

static int in(char c, const char* tests, size_t num_tests) {
    for (size_t i = 0; i < num_tests; i++) {
        if (c == tests[i]) {
            return 1;
        }
    }
    return 0;
}

void sv_strip(StringView* in_out) {
    static const char* strip_chars = " \n\r";
    size_t strip_chars_length = 3;

    while (in_out->length != 0) {
        if (in(*in_out->data, strip_chars, strip_chars_length)) {
            in_out->data++;
            in_out->length--;
        } else {
            break;
        }
    }
    while (in_out->length != 0) {
        if (in(*(in_out->data + in_out->length - 1), strip_chars, strip_chars_length)) {
            in_out->length--;
        } else {
            break;
        }
    }
}

bool sv_cmp(const StringView s1, const StringView s2, bool match_case) {
    if (s1.length != s2.length) {
        return false;
    }
    if (match_case) {
        return strncmp(s1.data, s2.data, s1.length) == 0;
    } else {
        for (size_t i = 0; i < s1.length; i++) {
            if (tolower((int)s1.data[i]) != tolower((int)s2.data[i])) {
                return false;
            }
        }
    }
    return true;
}
