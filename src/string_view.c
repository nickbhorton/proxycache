#include "string_view.h"

#include <string.h>

int sv_split_n(
    StringView* dest, size_t dest_count, const char* src, size_t src_length, const char* split,
    bool exact_amount
) {
    size_t split_length = strlen(split);
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
