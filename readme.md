# Design
1. Proxy Cache (PC) `listen` for clients.
2. A client makes a http request.
3. PC connects to client with `accept`.
4. PC spawns child to handle request.
5. Child receives the request
6. Request is parsed
6. Child validates request method is GET
    - If invalid sends client 400
7. Child validates origin server
    - If invalid sends client 404

# API
```c
int http_connect(
    const char* node
);
```
*parameters*:
node: the url to connect to, example "www.example.com"

*returns*:
On success it returns a socket file descriptor that is connected to `node`. On
failure a error message will be printed to `stderr` and -1 will be returned.

# System Calls Used

```c
int getaddrinfo(
    const char* nodename,
    const char* servname,
    const struct addrinfo* hints,
    struct addrinfo** res
);
```
`nodename` is the c string description of what you are connecting too. For
example "192.168.0.1" or "www.example.com". It is very generic. 

`servname` is the c string of the port number, or even better it can be the
string name of the service like "http".

`hints` allows specification of other things like the socket type, or the
address family (IPv4 or 6).

This function returns a list of pointers to filled out addrinfo structs.

