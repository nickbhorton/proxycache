#ifndef PROXY_CACHE_ADDRESS_HEADER
#define PROXY_CACHE_ADDRESS_HEADER

#include <sys/socket.h>

typedef struct {
    struct sockaddr_storage address;
    socklen_t address_length;
} Address;

#endif
