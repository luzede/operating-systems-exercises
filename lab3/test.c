#include <sys/wait.h> // Used to wait for child process to finish
#include <stdio.h> // printf and scanf, snprintf
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <time.h>


int main() {
    struct pollfd fds[1];
    int timeout_msecs = 20000;
    int ret;

    // Monitor stdin (fd 0) for input
    fds[0].fd = 0;
    fds[0].events = POLLIN;

    ret = poll(fds, 1, timeout_msecs);
    if (ret == -1) {
        perror("poll");
        return 1;
    }

    if (fds[0].revents & POLLIN) {
        printf("stdin is ready for reading\n");
        char input[20];
        printf("Bytes read %ld\n", read(0, input, sizeof(input)));
        char *end;
        long num;
        num = strtol(input, &end, 10);
        printf("%ld\n%s\n", num, end);
        printf("You entered: %s\n", input);
        if (end == input) {
          printf("hello world\n");
        }
    } else {
        printf("No data within five seconds.\n");
    }

    return 0;
}