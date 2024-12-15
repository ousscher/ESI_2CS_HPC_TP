#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <sys/time.h>


#define PI 3.14159265358979323846

void computeDFT(double* signal, int N, double complex* result) {
    for (int k = 0; k < N; k++) {
        double complex sum = 0.0 + 0.0 * I;
        for (int n = 0; n < N; n++) {
            double angle = 2 * PI * k * n / N;
            sum += signal[n] * cexp(-I * angle);
        }
        result[k] = sum;
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
        // Signal complexe = Partie réelle + Partie imaginaire
        // Imaginons que la partie réelle est un signal sinusoïdal de fréquence 50 Hz
        // Et la partie imaginaire est un signal sinusoïdal de fréquence 120 Hz
        double real_part = sin(2 * PI * 50 * i / N);   // Partie réelle du signal
        double imag_part = cos(2 * PI * 120 * i / N);  // Partie imaginaire du signal
        signal[i] = real_part + imag_part * I;  // Signal complexe
    }

    printf("Calcul de la DFT...\n");
    struct timeval start, end;
    gettimeofday(&start, NULL);
    computeDFT(signal, N, result);
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
