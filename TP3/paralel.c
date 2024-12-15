#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <sys/time.h>
#include <pthread.h>

#define PI 3.14159265358979323846
#define NUM_THREADS 4  

// Structure pour passer les arguments aux threads
typedef struct {
    double* signal;
    double complex* result;
    int start;
    int end;
    int N;
} ThreadArgs;

void* computeDFTThread(void* args) {
    ThreadArgs* targs = (ThreadArgs*)args;
    double* signal = targs->signal;
    double complex* result = targs->result;
    int start = targs->start;
    int end = targs->end;
    int N = targs->N;

    // Chaque thread calcule une partie de la DFT
    for (int k = start; k < end; k++) {
        double complex sum = 0.0 + 0.0 * I;
        for (int n = 0; n < N; n++) {
            double angle = 2 * PI * k * n / N;
            sum += signal[n] * cexp(-I * angle);
        }
        result[k] = sum;
    }

    pthread_exit(NULL);
}

void computeDFTParallel(double* signal, int N, double complex* result) {
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];
    int points_per_thread = N / NUM_THREADS;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].signal = signal;
        thread_args[i].result = result;
        thread_args[i].N = N;
        thread_args[i].start = i * points_per_thread;
        thread_args[i].end = (i == NUM_THREADS - 1) ? N : (i + 1) * points_per_thread;
        
        if (pthread_create(&threads[i], NULL, computeDFTThread, &thread_args[i]) != 0) {
            printf("Erreur lors de la création du thread %d\n", i);
            exit(1);
        }
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    int N = 10240;
    double* signal = (double*)malloc(N * sizeof(double));
    double complex* result = (double complex*)malloc(N * sizeof(double complex));

    if (signal == NULL || result == NULL) {
        printf("Erreur d'allocation mémoire.\n");
        return 1;
    }

    printf("Création du signal complexe...\n");
    for (int i = 0; i < N; i++) {
        double real_part = sin(2 * PI * 50 * i / N);
        double imag_part = cos(2 * PI * 120 * i / N);
        signal[i] = real_part + imag_part * I;
    }

    printf("Calcul de la DFT parallèle...\n");
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    computeDFTParallel(signal, N, result);
    
    gettimeofday(&end, NULL);
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Temps d'exécution : %f secondes\n", time_spent);

    printf("Résultats de la DFT (partiels) :\n");
    for (int k = 0; k < 10; k++) {
        printf("k = %d : Magnitude = %.5f, Phase = %.5f radians\n", k, cabs(result[k]), carg(result[k]));
    }

    free(signal);
    free(result);

    return 0;
}