#ifndef PROXY_CACHE_CONNECTION_HEADER
#define PROXY_CACHE_CONNECTION_HEADER

#include "address.h"

typedef struct {
    int fd;
    Address address;
} Connection;

Connection pc_accept(int listening_socket);

void pc_handle_connection(Connection* c);

#endif
