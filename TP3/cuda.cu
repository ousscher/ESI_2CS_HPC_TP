%%cuda
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <cuda_runtime.h>
#include <cuComplex.h>

#define PI 3.14159265358979323846
#define BLOCK_SIZE 256

typedef struct {
    double real;
    double imag;
} ComplexNumber;

// Kernel CUDA pour le calcul de la TFD
__global__ void dftKernel(double* signal_real, double* signal_imag, 
                         cuDoubleComplex* result, int N) {
    int k = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (k < N) {
        cuDoubleComplex sum = make_cuDoubleComplex(0.0, 0.0);
        
        for (int n = 0; n < N; n++) {
            double angle = 2 * PI * k * n / N;
            double cos_angle = cos(angle);
            double sin_angle = sin(angle);
            
            // Multiplication complexe
            double real_part = signal_real[n] * cos_angle + signal_imag[n] * sin_angle;
            double imag_part = signal_imag[n] * cos_angle - signal_real[n] * sin_angle;
            
            sum = cuCadd(sum, make_cuDoubleComplex(real_part, -imag_part));
        }
        
        result[k] = sum;
    }
}

void checkCudaError(cudaError_t error, const char *message) {
    if (error != cudaSuccess) {
        fprintf(stderr, "CUDA Error: %s - %s\n", message, cudaGetErrorString(error));
        exit(EXIT_FAILURE);
    }
}

void computeDFT_CUDA(ComplexNumber* signal, int N, ComplexNumber* result) {
    double *d_signal_real, *d_signal_imag;
    cuDoubleComplex *d_result;
    
    double* signal_real = (double*)malloc(N * sizeof(double));
    double* signal_imag = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        signal_real[i] = signal[i].real;
        signal_imag[i] = signal[i].imag;
    }
    checkCudaError(cudaMalloc((void**)&d_signal_real, N * sizeof(double)), 
                  "Allocation signal réel");
    checkCudaError(cudaMalloc((void**)&d_signal_imag, N * sizeof(double)), 
                  "Allocation signal imaginaire");
    checkCudaError(cudaMalloc((void**)&d_result, N * sizeof(cuDoubleComplex)), 
                  "Allocation résultat");
    
    // Copie des données vers le GPU
    checkCudaError(cudaMemcpy(d_signal_real, signal_real, N * sizeof(double), 
                  cudaMemcpyHostToDevice), "Copie signal réel vers GPU");
    checkCudaError(cudaMemcpy(d_signal_imag, signal_imag, N * sizeof(double), 
                  cudaMemcpyHostToDevice), "Copie signal imaginaire vers GPU");
    
    // Configuration de la grille et des blocs
    int numBlocks = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;
    dim3 gridDim(numBlocks);
    dim3 blockDim(BLOCK_SIZE);
    
    // Lancement du kernel
    dftKernel<<<gridDim, blockDim>>>(d_signal_real, d_signal_imag, d_result, N);
    
    // Vérification des erreurs du kernel
    checkCudaError(cudaGetLastError(), "Lancement du kernel");
    checkCudaError(cudaDeviceSynchronize(), "Synchronisation du kernel");
    
    // Copie des résultats vers le CPU
    cuDoubleComplex* h_result = (cuDoubleComplex*)malloc(N * sizeof(cuDoubleComplex));
    checkCudaError(cudaMemcpy(h_result, d_result, N * sizeof(cuDoubleComplex), 
                  cudaMemcpyDeviceToHost), "Copie résultats vers CPU");
    
    // Conversion des résultats en format ComplexNumber
    for (int i = 0; i < N; i++) {
        result[i].real = h_result[i].x;
        result[i].imag = h_result[i].y;
    }
    
    // Libération de la mémoire
    free(signal_real);
    free(signal_imag);
    free(h_result);
    cudaFree(d_signal_real);
    cudaFree(d_signal_imag);
    cudaFree(d_result);
}

int main() {
    int N = 10240;
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
    
    printf("Calcul de la DFT avec CUDA...\n");
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    computeDFT_CUDA(signal, N, result);
    
    gettimeofday(&end, NULL);
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + 
                       (end.tv_usec - start.tv_usec) / 1e6;
    
    printf("Temps d'exécution CUDA : %f secondes\n", time_spent);
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