/*
 * fibonacci.c
 * Usage: ./fibonacci <N>
 * Creates a worker thread that fills an array with the first N Fibonacci numbers.
 * The main thread waits with pthread_join and then prints the array.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    unsigned long long *arr;
    long long N;
} FibArgs;

void *fib_worker(void *arg)
{
    FibArgs *a = (FibArgs *)arg;
    unsigned long long *arr = a->arr;
    long long N = a->N;

    if (N <= 0) return NULL;
    arr[0] = 0ULL;
    if (N == 1) return NULL;
    arr[1] = 1ULL;
    for (long long i = 2; i < N; ++i) {
        arr[i] = arr[i-1] + arr[i-2];
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    long long N = atoll(argv[1]);
    if (N < 0) {
        fprintf(stderr, "N must be non-negative\n");
        return 1;
    }

    unsigned long long *arr = malloc(sizeof(unsigned long long) * (N > 0 ? N : 1));
    if (!arr) {
        perror("malloc");
        return 1;
    }

    FibArgs args = { .arr = arr, .N = N };
    pthread_t thread;
    if (pthread_create(&thread, NULL, fib_worker, &args) != 0) {
        perror("pthread_create");
        free(arr);
        return 1;
    }

    if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join");
        free(arr);
        return 1;
    }

    for (long long i = 0; i < N; ++i) {
        if (i) printf(" ");
        printf("%llu", (unsigned long long)arr[i]);
    }
    if (N > 0) printf("\n");

    free(arr);
    return 0;
}
