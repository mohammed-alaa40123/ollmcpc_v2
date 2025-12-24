#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        printf("parent waiting for child........\n");
        wait(NULL); // makes parent waits for child process to finish
        printf("child reaped no zombie remains.\n");
    } else {
        printf("child exiting.\n");
        _exit(0);
    }
    return 0;
}
