#ifndef PROXY_CACHE_STRING_VIEW_HEADER
#define PROXY_CACHE_STRING_VIEW_HEADER

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char* data;
    size_t length;
} StringView;

/*! @fn int sv_split_n
 * @brief Splits src into StringViews
 *
 * Take in a c string @a src of length @a src_length and splits it on the split c string. Returns
 * the array of split strings via the out variable @a dest with a max number @a dest_count.
 *
 * @param[out] dest out variable that function puts the substrings into
 * @param dest_count maximum amount of substrings function can put into dest
 * @param src the c string the function will break into parts
 * @param src_length size of src
 * @param split the char(s) the function should split on
 * @param exact_amount specifies if the split c string can be repeated.
 *
 * @return number of StringViews put into dest
 */
int sv_split_n(
    StringView* dest, size_t dest_count, const char* src, size_t src_length, const char* split,
    bool exact_amount
);

void sv_strip(StringView* in_out);

bool sv_cmp(StringView s1, StringView s2, bool match_case);

#endif
