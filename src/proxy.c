#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "connection.h"
#include "tcp.h"

#define FailIsFatal(call)                                                                          \
    if ((call) < 0) {                                                                              \
        fflush(stdout);                                                                            \
        fflush(stderr);                                                                            \
        exit(EXIT_FAILURE);                                                                        \
    }

#define DebugPrint(...) fprintf(stdout, __VA_ARGS__);

int PC_TIMEOUT = 10;

int proxy_fd;

void pc_sigint(int signal) {
    shutdown(proxy_fd, 2);
    fflush(stdout);
    fflush(stderr);
    exit(0);
}
void usage();

int main(int argc, char** argv) {
    // setup
    if (argc < 3) {
        usage();
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port < 1024) {
        printf("port was wrong\n");
        usage();
        exit(EXIT_FAILURE);
    }
    PC_TIMEOUT = atoi(argv[2]);
    if (PC_TIMEOUT <= 0) {
        printf("timeout was wrong\n");
        usage();
        exit(EXIT_FAILURE);
    }

    // Release control of children to prevent zombies
    signal(SIGCHLD, SIG_IGN);

    // making the cache dir defensivly
    struct stat st = {0};
    if (stat("./cache", &st) == -1) {
        mkdir("./cache", 0700);
    }

    // handle SIGINT
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = pc_sigint;
    FailIsFatal(sigaction(SIGINT, &sa, NULL));

    FailIsFatal(proxy_fd = tcp_connect(NULL, port, true));
    FailIsFatal(listen(proxy_fd, 128));

    // handle connections loop
    while (1) {
        Connection client_connection = pc_accept(proxy_fd);
        if (client_connection.fd < 0) {
            DebugPrint("pc_accept error, continuing\n");
            continue;
        }
        if (!fork()) {
            close(proxy_fd);
            int rv = pc_handle_connection(&client_connection);
            if (rv != 0) {
                printf("%d %d\n", getpid(), rv);
            }
            // clean up connection
            shutdown(client_connection.fd, 2);
            exit(0);
        } else {
            close(client_connection.fd);
        }
    }
}

void usage() { printf("./proxy <port> <timeout>\n"); }
