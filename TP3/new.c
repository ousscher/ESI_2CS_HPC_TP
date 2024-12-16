#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846
typedef struct {
    double real;
    double imag;
} Complex;

void computeDFT(const Complex* signal, Complex* dft_result, int N) {
    for (int k = 0; k < N; ++k) {
        dft_result[k].real = 0.0;
        dft_result[k].imag = 0.0;

        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * PI * k * n / N;
            dft_result[k].real += signal[n].real * cos(angle) + signal[n].imag * sin(angle);
            dft_result[k].imag += signal[n].imag * cos(angle) - signal[n].real * sin(angle);
        }
    }
}

int main() {
    int N = 10240;

    // Allocation de mémoire pour le signal et le résultat
    Complex* signal = (Complex*)malloc(N * sizeof(Complex));
    Complex* dft_result = (Complex*)malloc(N * sizeof(Complex));
    if (signal == NULL || dft_result == NULL) {
        printf("Erreur d'allocation de mémoire.\n");
        return -1;
    }

    // Création du signal
    for (int n = 0; n < N; ++n) {
        signal[n].real = sin(2 * PI * 50 * n / N);
        signal[n].imag = cos(2 * PI * 120 * n / N);
    }
    computeDFT(signal, dft_result, N);
    for (int k = 0; k < 10; ++k) {
        double magnitude = sqrt(dft_result[k].real * dft_result[k].real + dft_result[k].imag * dft_result[k].imag);
        double phase = atan2(dft_result[k].imag, dft_result[k].real);
        printf("k = %d: Magnitude = %.5f, Phase = %.5f radians\n", k, magnitude, phase);
    }
    free(signal);
    free(dft_result);

    return 0;
}
