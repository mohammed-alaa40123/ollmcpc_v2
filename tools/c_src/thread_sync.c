#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct counter_state {
  long long value;
  long long loops;
  pthread_mutex_t mutex;
};

static void *incrementer(void *arg) {
  struct counter_state *state = (struct counter_state *)arg;
  for (long long i = 0; i < state->loops; i++) {
    pthread_mutex_lock(&state->mutex);
    state->value++;
    pthread_mutex_unlock(&state->mutex);
  }
  return NULL;
}

int main(void) {
  const int threads_count = 4;
  const long long loops = 50000;

  struct counter_state state;
  state.value = 0;
  state.loops = loops;
  if (pthread_mutex_init(&state.mutex, NULL) != 0) {
    perror("pthread_mutex_init");
    return 1;
  }

  pthread_t threads[threads_count];
  for (int i = 0; i < threads_count; i++) {
    if (pthread_create(&threads[i], NULL, incrementer, &state) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (int i = 0; i < threads_count; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      perror("pthread_join");
      return 1;
    }
  }

  pthread_mutex_destroy(&state.mutex);

  long long expected = loops * threads_count;
  printf("Final counter: %lld\n", state.value);
  printf("Expected: %lld\n", expected);
  return 0;
}
