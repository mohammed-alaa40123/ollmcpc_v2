#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct thread_info {
  const char *name;
  int loops;
};

static void *run_role(void *arg) {
  struct thread_info *info = (struct thread_info *)arg;
  for (int i = 1; i <= info->loops; i++) {
    printf("%s thread %lu iteration %d\n", info->name,
           (unsigned long)pthread_self(), i);
    fflush(stdout);
    usleep(100000);
  }
  return NULL;
}

int main(void) {
  pthread_t threads[3];
  struct thread_info roles[3] = {
      {"worker", 3},
      {"logger", 3},
      {"monitor", 3},
  };

  for (int i = 0; i < 3; i++) {
    if (pthread_create(&threads[i], NULL, run_role, &roles[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < 3; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("pthread_join");
      return 1;
    }
  }

  printf("All threads completed.\n");
  return 0;
}
