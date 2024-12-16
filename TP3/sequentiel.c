#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define PI 3.14159265358979323846

// Structure pour stocker les nombres complexes
typedef struct {
    double real;
    double imag;
} ComplexNumber;

// Fonction pour multiplier deux nombres complexes
ComplexNumber multiplyComplex(ComplexNumber a, ComplexNumber b) {
    ComplexNumber result;
    result.real = a.real * b.real - a.imag * b.imag;
    result.imag = a.real * b.imag + a.imag * b.real;
    return result;
}

// Fonction pour additionner deux nombres complexes
ComplexNumber addComplex(ComplexNumber a, ComplexNumber b) {
    ComplexNumber result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

// Fonction pour calculer la TFD de manière séquentielle
void computeDFT(ComplexNumber* signal, int N, ComplexNumber* result) {
    for (int k = 0; k < N; k++) {
        result[k].real = 0;
        result[k].imag = 0;
        
        for (int n = 0; n < N; n++) {
            double angle = 2 * PI * k * n / N;
            
            // Calcul de e^(-j*2π*k*n/N)
            ComplexNumber exponential = {
                .real = cos(angle),
                .imag = -sin(angle)
            };
            
            // Multiplication avec le signal
            ComplexNumber temp = multiplyComplex(signal[n], exponential);
            
            // Accumulation du résultat
            result[k] = addComplex(result[k], temp);
        }
    }
}

int main() {
    int N = 10240;  // Même taille que dans la version CUDA
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
    
    printf("Calcul de la DFT séquentielle...\n");
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    computeDFT(signal, N, result);
    
    gettimeofday(&end, NULL);
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + 
                       (end.tv_usec - start.tv_usec) / 1e6;
    
    printf("Temps d'exécution séquentiel : %f secondes\n", time_spent);
    printf("Résultats de la DFT (partiels) :\n");
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