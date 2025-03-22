#include "url.h"

#include "string_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_url(const char* url, size_t url_length, Url* dest) {
    StringView scheme_pass[2];
    int rv = sv_split_n(scheme_pass, 2, url, url_length, "://", false);
    if (rv != 2) {
        return -1;
    }
    dest->scheme = scheme_pass[0];

    StringView para_anch_pass[2];
    rv = sv_split_n(para_anch_pass, 2, scheme_pass[1].data, scheme_pass[1].length, "?", false);
    if (rv == 2) {
        dest->parameters_anchor = para_anch_pass[1];
    }

    StringView authoritiy_pass[2];
    rv = sv_split_n(
        authoritiy_pass, 2, para_anch_pass[0].data, para_anch_pass[0].length, "/", false
    );
    if (rv == 2) {
        dest->path = authoritiy_pass[1];
        // this is a bit sketch
        dest->path.length = para_anch_pass[0].length - authoritiy_pass[0].length - 1;
    }

    StringView port_pass[2];
    rv = sv_split_n(port_pass, 2, authoritiy_pass[0].data, authoritiy_pass[0].length, ":", true);
    if (rv == 1) {
        dest->port = 80;
    } else if (rv == 2) {
        char port_buff[16];
        memset(port_buff, 0, 16);
        // don't buffer overflow from user input
        memcpy(port_buff, port_pass[1].data, port_pass[1].length > 16 ? 16 : port_pass[1].length);
        dest->port = atoi(port_buff);
    }
    dest->domain = port_pass[0];

    return 0;
}
