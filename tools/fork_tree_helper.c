#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("parent pid: %d \n", getpid());

    pid_t child1 = fork();
    if (child1 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (child1 == 0) {
        // first child
        printf("child1 pid: %d, parent: %d \n", getpid(), getppid());

        pid_t grandchild1 = fork();
        if (grandchild1 < 0) {
            perror("fork failed");
            exit(1);
        }

        if (grandchild1 == 0) {
            // first grandchild
            printf("grandchild1 pid: %d, parent: %d \n", getpid(), getppid());
            sleep(2);
            exit(0);
        }

        pid_t grandchild2 = fork();
        if (grandchild2 < 0) {
            perror("fork failed");
            exit(1);
        }

        if (grandchild2 == 0) {
            printf("grandchild2 pid: %d, parent: %d 🌿\n", getpid(), getppid());
            sleep(2);
            exit(0);
        }

        wait(NULL);
        wait(NULL);
        sleep(1);
        exit(0);
    }

    pid_t child2 = fork();
    if (child2 < 0) {
        perror("fork failed");
        exit(1);
    }

    if (child2 == 0) {
        printf("child2 pid: %d, parent: %d \n", getpid(), getppid());
        sleep(3);
        exit(0);
    }

    wait(NULL);
    wait(NULL);

    sleep(1);
    return 0;
}
