#ifndef PROXY_CACHE_CONNECTION_HEADER
#define PROXY_CACHE_CONNECTION_HEADER

#include "address.h"
#include "url.h"

#define PC_TIMEOUT_MS 1000
#define PC_BUFFER_SIZE 4096

typedef struct {
    int fd;
    Address address;
} Connection;

Connection pc_accept(int listening_socket);

void pc_handle_connection(Connection* c);

// returns a file descriptor of requested file
int pc_get_file(const StringView proxy_request, const Url* url, size_t* file_size_o);

#endif
