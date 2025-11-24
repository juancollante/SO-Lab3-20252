/*
 * Parallel Pi calculation using Pthreads
 * Usage: ./pi_p <n_intervals> <num_threads>
 * Compilar: gcc -o pi_p pi_p.c -lpthread -lm
 */

/* Expose POSIX clock functions like clock_gettime and CLOCK_MONOTONIC */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    long long start;
    long long end;
    double fH;
    double partial_sum;
} ThreadArg;

static inline double f(double a)
{
    return (4.0 / (1.0 + a*a));
}

void *thread_func(void *arg)
{
    ThreadArg *t = (ThreadArg *)arg;
    double sum = 0.0;
    for (long long i = t->start; i < t->end; ++i) {
        double x = t->fH * ((double)i + 0.5);
        sum += f(x);
    }
    t->partial_sum = sum;
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <n_intervals> <num_threads>\n", argv[0]);
        return 1;
    }

    long long n = atoll(argv[1]);
    int T = atoi(argv[2]);

    if (n <= 0 || T <= 0) {
        fprintf(stderr, "n and T must be positive integers\n");
        return 1;
    }

    if (n > 0x7FFFFFFF) {
        /* keep consistent with original pi.c which used int */
    }

    ThreadArg *args = malloc(sizeof(ThreadArg) * T);
    if (!args) {
        perror("malloc");
        return 1;
    }
    pthread_t *threads = malloc(sizeof(pthread_t) * T);
    if (!threads) {
        perror("malloc");
        free(args);
        return 1;
    }

    const double fH = 1.0 / (double)n;

    long long chunk = n / T;

    struct timespec tstart, tend;
    if (clock_gettime(CLOCK_MONOTONIC, &tstart) != 0) {
        perror("clock_gettime");
        free(args);
        free(threads);
        return 1;
    }

    for (int j = 0; j < T; ++j) {
        long long start = (long long)j * chunk;
        long long end = (j == T-1) ? n : start + chunk;
        args[j].start = start;
        args[j].end = end;
        args[j].fH = fH;
        args[j].partial_sum = 0.0;
        if (pthread_create(&threads[j], NULL, thread_func, &args[j]) != 0) {
            perror("pthread_create");
            /* join already created threads */
            for (int k = 0; k < j; ++k) pthread_join(threads[k], NULL);
            free(args);
            free(threads);
            return 1;
        }
    }

    double total = 0.0;
    for (int j = 0; j < T; ++j) {
        pthread_join(threads[j], NULL);
        total += args[j].partial_sum;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &tend) != 0) {
        perror("clock_gettime");
        free(args);
        free(threads);
        return 1;
    }

    double pi = fH * total;
    double elapsed = (double)(tend.tv_sec - tstart.tv_sec) +
                     (double)(tend.tv_nsec - tstart.tv_nsec) / 1e9;

    const double fPi25DT = 3.141592653589793238462643;
    printf("\npi is approximately = %.20f \nError               = %.20f\n",
           pi, fabs(pi - fPi25DT));
    printf("Threads: %d\n", T);
    printf("Time (seconds): %.9f\n", elapsed);

    free(args);
    free(threads);
    return 0;
}
