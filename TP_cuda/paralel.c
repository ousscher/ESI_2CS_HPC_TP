#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define PI 3.14159265358979323846

typedef struct {
    double real;
    double imag;
} ComplexNumber;

typedef struct {
    ComplexNumber* signal;
    ComplexNumber* result;
    int N;
    int start;
    int end;
} ThreadData;

ComplexNumber multiplyComplex(ComplexNumber a, ComplexNumber b) {
    ComplexNumber result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

ComplexNumber addComplex(ComplexNumber a, ComplexNumber b) {
    ComplexNumber result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

void* computeDFTThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    ComplexNumber* signal = data->signal;
    ComplexNumber* result = data->result;
    int N = data->N;

    for (int k = data->start; k < data->end; k++) {
        result[k].real = 0;
        result[k].imag = 0;

        for (int n = 0; n < N; n++) {
            double angle = 2 * PI * k * n / N;
            ComplexNumber exponential = {
                .real = cos(angle),
                .imag = -sin(angle)
            };

            ComplexNumber temp = multiplyComplex(signal[n], exponential);
            result[k] = addComplex(result[k], temp);
        }
    }

    return NULL;
}

int main() {
    int N = 10240;  // Taille du signal
    int numThreads = 8; 

    ComplexNumber* signal = (ComplexNumber*)malloc(N * sizeof(ComplexNumber));
    ComplexNumber* result = (ComplexNumber*)malloc(N * sizeof(ComplexNumber));

    if (signal == NULL || result == NULL) {
        printf("Erreur d'allocation mémoire.\n");
        return 1;
    }

    printf("Création du signal complexe...\n");
    for (int i = 0; i < N; i++) {
        signal[i].real = sin(2 * PI * 50 * i / N);
        signal[i].imag = cos(2 * PI * 120 * i / N);
    }

    printf("Calcul de la DFT parallèle avec %d threads...\n", numThreads);
    struct timeval start, end;
    gettimeofday(&start, NULL);

    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];

    int chunkSize = N / numThreads;
    for (int t = 0; t < numThreads; t++) {
        threadData[t].signal = signal;
        threadData[t].result = result;
        threadData[t].N = N;
        threadData[t].start = t * chunkSize;
        threadData[t].end = (t == numThreads - 1) ? N : (t + 1) * chunkSize;

        pthread_create(&threads[t], NULL, computeDFTThread, &threadData[t]);
    }

    for (int t = 0; t < numThreads; t++) {
        pthread_join(threads[t], NULL);
    }

    gettimeofday(&end, NULL);
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + 
                        (end.tv_usec - start.tv_usec) / 1e6;

    printf("Temps d'ex\u00e9cution parall\u00e8le : %f secondes\n", time_spent);
    printf("R\u00e9sultats de la DFT (partiels) :\n");
    for (int k = 0; k < 10; k++) {
        double magnitude = sqrt(result[k].real * result[k].real + 
                                result[k].imag * result[k].imag);
        double phase = atan2(result[k].imag, result[k].real);
        printf("k = %d : Magnitude = %.5f, Phase = %.5f radians\n", 
               k, magnitude, phase);
    }

    free(signal);
    free(result);

    return 0;
}
