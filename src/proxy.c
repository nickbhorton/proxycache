#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connection.h"
#include "utils.h"

#define FailIsFatal(call)                                                                          \
    if ((call) < 0) {                                                                              \
        fflush(stdout);                                                                            \
        fflush(stderr);                                                                            \
        exit(EXIT_FAILURE);                                                                        \
    }

#define DebugPrint(...) fprintf(stdout, __VA_ARGS__);

int proxy_fd;

void pc_sigint(int signal) {
    shutdown(proxy_fd, 2);
    fflush(stdout);
    fflush(stderr);
    exit(0);
}

int main() {
    // Release control of children to prevent zombies
    signal(SIGCHLD, SIG_IGN);

    // handle SIGINT
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = pc_sigint;
    FailIsFatal(sigaction(SIGINT, &sa, NULL));

    FailIsFatal(proxy_fd = tcp_connect(NULL, "8888", true));
    FailIsFatal(listen(proxy_fd, 128));

    while (1) {
        Connection client_connection = pc_accept(proxy_fd);
        if (client_connection.fd < 0) {
            DebugPrint("pc_accept error, continuing\n");
            continue;
        }
        if (!fork()) {
            close(proxy_fd);
            pc_handle_connection(&client_connection);
            // shuting down connection
            shutdown(client_connection.fd, 2);
            exit(0);
        } else {
            close(client_connection.fd);
        }
    }
}
