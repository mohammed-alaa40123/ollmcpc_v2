#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t got_usr1 = 0;
static volatile sig_atomic_t got_usr2 = 0;

static void on_usr1(int sig) {
  (void)sig;
  got_usr1 = 1;
}

static void on_usr2(int sig) {
  (void)sig;
  got_usr2 = 1;
}

static int parse_rounds(int argc, char **argv) {
  if (argc < 2) {
    return 10;
  }
  char *end = NULL;
  long value = strtol(argv[1], &end, 10);
  if (end == argv[1] || *end != '\0' || value <= 0) {
    fprintf(stderr, "Invalid rounds: %s\n", argv[1]);
    return -1;
  }
  return (int)value;
}

int main(int argc, char **argv) {
  int rounds = parse_rounds(argc, argv);
  if (rounds <= 0) {
    return 1;
  }

  struct sigaction sa1;
  struct sigaction sa2;
  sa1.sa_handler = on_usr1;
  sa2.sa_handler = on_usr2;
  sigemptyset(&sa1.sa_mask);
  sigemptyset(&sa2.sa_mask);
  sa1.sa_flags = 0;
  sa2.sa_flags = 0;

  if (sigaction(SIGUSR1, &sa1, NULL) != 0) {
    perror("sigaction SIGUSR1");
    return 1;
  }
  if (sigaction(SIGUSR2, &sa2, NULL) != 0) {
    perror("sigaction SIGUSR2");
    return 1;
  }

  sigset_t block_set;
  sigset_t wait_mask;
  sigemptyset(&block_set);
  sigaddset(&block_set, SIGUSR1);
  sigaddset(&block_set, SIGUSR2);
  if (sigprocmask(SIG_BLOCK, &block_set, &wait_mask) != 0) {
    perror("sigprocmask");
    return 1;
  }
  sigdelset(&wait_mask, SIGUSR1);
  sigdelset(&wait_mask, SIGUSR2);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return 1;
  }

  if (pid == 0) {
    pid_t parent = getppid();
    for (int round = 1; round <= rounds; round++) {
      got_usr1 = 0;
      while (!got_usr1) {
        sigsuspend(&wait_mask);
      }
      printf("child %ld received SIGUSR1 round %d\n", (long)getpid(), round);
      fflush(stdout);
      if (kill(parent, SIGUSR2) != 0) {
        perror("kill SIGUSR2");
        _exit(1);
      }
    }
    _exit(0);
  }

  for (int round = 1; round <= rounds; round++) {
    got_usr2 = 0;
    if (kill(pid, SIGUSR1) != 0) {
      perror("kill SIGUSR1");
      break;
    }
    while (!got_usr2) {
      sigsuspend(&wait_mask);
    }
    printf("parent %ld received SIGUSR2 round %d\n", (long)getpid(), round);
    fflush(stdout);
  }

  waitpid(pid, NULL, 0);
  return 0;
}
