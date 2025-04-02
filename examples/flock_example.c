#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>

const char* msg = "hello world\n";

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("specify a parent timeout\n");
        exit(EXIT_FAILURE);
    }

    size_t msg_len = strlen(msg);
    static char rd_buffer[13];
    rd_buffer[12] = '\0';

    if (!fork()) {
        int fd = open("test.txt", O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
        if (fd < 0) {
            perror("child open");
            while (1) {
                fd = open("test.txt", O_RDONLY);
                if (fd < 0) {
                    perror("child open for reading");
                } else {
                    break;
                }
                sleep(1);
            }

            // shared lock the file
            int rv = flock(fd, LOCK_SH);
            if (rv < 0) {
                perror("child flock lock_sh");
                exit(EXIT_FAILURE);
            }

            // read the file
            int bytes_read = read(fd, rd_buffer, msg_len);
            assert(bytes_read == msg_len);
            printf("child read: %s", rd_buffer);

            // unlock file
            rv = flock(fd, LOCK_UN);
            if (rv < 0) {
                perror("child flock lock_sh");
                exit(EXIT_FAILURE);
            }
        } else {
            // exclusive lock the file
            int rv = flock(fd, LOCK_EX);
            if (rv < 0) {
                perror("child flock lock");
                exit(EXIT_FAILURE);
            }
            printf("file locked\n");

            // write something to file
            int bytes_written = write(fd, msg, msg_len);
            assert(bytes_written == msg_len);
            printf("file written\n");

            // unlock file
            rv = flock(fd, LOCK_UN);
            if (rv < 0) {
                perror("child flock unlock");
                exit(EXIT_FAILURE);
            }
            printf("file unlocked\n");
        }
    } else {
        usleep(atoi(argv[1]));
        int fd = open("test.txt", O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
        if (fd < 0) {
            perror("parent open");
            while (1) {
                fd = open("test.txt", O_RDONLY);
                if (fd < 0) {
                    perror("parent open for reading");
                } else {
                    break;
                }
                sleep(1);
            }

            // shared lock the file
            int rv = flock(fd, LOCK_SH);
            if (rv < 0) {
                perror("parent flock lock_sh");
                exit(EXIT_FAILURE);
            }

            // read the file
            int bytes_read = read(fd, rd_buffer, msg_len);
            assert(bytes_read == msg_len);
            printf("parent read: %s", rd_buffer);

            // unlock file
            rv = flock(fd, LOCK_UN);
            if (rv < 0) {
                perror("parent flock lock_sh");
                exit(EXIT_FAILURE);
            }
        } else {
            // exclusive lock the file
            int rv = flock(fd, LOCK_EX);
            if (rv < 0) {
                perror("parent flock lock");
                exit(EXIT_FAILURE);
            }
            printf("file locked\n");

            // write something to file
            int bytes_written = write(fd, msg, msg_len);
            assert(bytes_written == msg_len);
            printf("file written\n");

            // unlock file
            rv = flock(fd, LOCK_UN);
            if (rv < 0) {
                perror("parent flock unlock");
                exit(EXIT_FAILURE);
            }
            printf("file unlocked\n");
        }

        // wait for child to finish
        wait(NULL);
    }
    fflush(stdout);
    return 0;
}
