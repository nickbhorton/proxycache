#include "filename.h"

#include <sys/stat.h>

#include "md5.h"

ssize_t get_filesize(const char* filename) {
    struct stat st;
    int rv = stat(filename, &st);
    if (rv < 0) {
        perror("stat");
        return rv;
    }
    return st.st_size;
}

void get_filename(const Url* url, char filename_o[39]) {
    static char hash_buffer[512];
    static uint8_t file_hash[16];
    snprintf(
        hash_buffer, 512, "%.*s/%.*s", (int)url->domain.length, url->domain.data,
        (int)url->path.length, url->path.data
    );
    md5String(hash_buffer, file_hash);
    snprintf(
        filename_o, 39, "cache/%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        file_hash[0], file_hash[1], file_hash[2], file_hash[3], file_hash[4], file_hash[5],
        file_hash[6], file_hash[7], file_hash[8], file_hash[9], file_hash[10], file_hash[11],
        file_hash[12], file_hash[13], file_hash[14], file_hash[15]
    );
}
