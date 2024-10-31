#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2
#define NUM_THREADS 2

typedef struct { 
    int thread_id;  
    int start_i;
    int end_i;
    int lenX;
    int lenY;
    char* X;
    char* Y;
    int** S;
} ThreadData;

void* calculate_wavefront(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    for (int d = 1; d <= data->lenX + data->lenY; d++) {
        for (int i = data->start_i; i <= data->end_i; i++) {
            int j = d - i;
            if (j < 1 || j > data->lenY) continue;

            int match = data->S[i - 1][j - 1] + ((data->X[i - 1] == data->Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
            int del = data->S[i - 1][j] + GAP_PENALTY;
            int insert = data->S[i][j - 1] + GAP_PENALTY;
            data->S[i][j] = fmax(fmax(match, del), insert);
        }
    }
    return NULL;
}

void calculate_similarity_matrix_parallel(char* X, char* Y, int lenX, int lenY, int** S) {
    for (int i = 0; i <= lenX; i++) S[i][0] = i * GAP_PENALTY;
    for (int j = 0; j <= lenY; j++) S[0][j] = j * GAP_PENALTY;

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int chunk_size = lenX / NUM_THREADS;

    for (int t = 0; t < NUM_THREADS; t++) {
        thread_data[t].thread_id = t; 
        thread_data[t].start_i = t * chunk_size + 1;
        thread_data[t].end_i = (t == NUM_THREADS - 1) ? lenX : (t + 1) * chunk_size;
        thread_data[t].lenX = lenX;
        thread_data[t].lenY = lenY;
        thread_data[t].X = X;
        thread_data[t].Y = Y;
        thread_data[t].S = S;

        pthread_create(&threads[t], NULL, calculate_wavefront, &thread_data[t]);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
}

void print_matrix(int lenX, int lenY, int** S) {
    for (int i = 0; i <= lenX; i++) {
        for (int j = 0; j <= lenY; j++) {
            printf("%3d ", S[i][j]); 
        }
        printf("\n");
    }
}

void traceback(int** S, char* X, char* Y, int lenX, int lenY) {
    char* aligned_X = (char*)malloc((lenX + lenY + 1) * sizeof(char)); 
    char* aligned_Y = (char*)malloc((lenX + lenY + 1) * sizeof(char)); 
    int index = 0; 

    int i = lenX;
    int j = lenY;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && S[i][j] == S[i - 1][j - 1] + ((X[i - 1] == Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE)) {
            aligned_X[index] = X[i - 1];
            aligned_Y[index] = Y[j - 1];
            i--;
            j--;
        } else if (i > 0 && S[i][j] == S[i - 1][j] + GAP_PENALTY) {
            aligned_X[index] = X[i - 1];
            aligned_Y[index] = '-';
            i--;
        } else {
            aligned_X[index] = '-';
            aligned_Y[index] = Y[j - 1];
            j--;
        }
        index++;
    }

    aligned_X[index] = '\0'; 
    aligned_Y[index] = '\0'; 

    printf("Alignement Optimal :\n");
    for (int k = index - 1; k >= 0; k--) {
        printf("%c", aligned_X[k]);
    }
    printf("\n");
    for (int k = index - 1; k >= 0; k--) {
        printf("%c", aligned_Y[k]);
    }
    printf("\n");
    free(aligned_X);
    free(aligned_Y);
}

void read_sequence_from_file(const char* filename, char** sequence, int* length) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        exit(1);
    }
    //trouver la taille du fichier
    fseek(file, 0, SEEK_END); 
    *length = ftell(file);
    printf("Taille du fichier %s : %d\n", filename, *length);
    rewind(file); 

    *sequence = (char*)malloc((*length + 1) * sizeof(char)); 
    if (*sequence == NULL) {
        fprintf(stderr, "Erreur : Impossible d'allouer la mémoire\n");
        exit(1);
    }

    fread(*sequence, sizeof(char), *length, file); 
    (*sequence)[*length] = '\0'; 
    fclose(file);
}


int main() {

    // char X[] = "ATA";  
    // char Y[] = "AGTTA"; 
    // char X[] = "AGCTGACGTAAGCTAGCTA";  
    // char Y[] = "GCTAGCAGTAGCAGTACGTA";  
    // int lenX = sizeof(X) / sizeof(X[0]) - 1; 
    // int lenY = sizeof(Y) / sizeof(Y[0]) - 1; 
    char *X, *Y;
    int lenX, lenY; 
    read_sequence_from_file("X.txt", &X, &lenX);
    read_sequence_from_file("Y.txt", &Y, &lenY);

    int** S = (int**)malloc((lenX + 1) * sizeof(int*));
    for (int i = 0; i <= lenX; i++) {
        S[i] = (int*)malloc((lenY + 1) * sizeof(int));
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);
    calculate_similarity_matrix_parallel(X, Y, lenX, lenY, S);
    gettimeofday(&end, NULL);
    // traceback(S, X, Y, lenX, lenY);
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6; // Calcul du temps écoulé
    printf("Temps d'exécution (parallèle) : %f secondes\n", time_spent);

    // print_matrix(lenX, lenY, S);
    traceback(S, X, Y, lenX, lenY);
    for (int i = 0; i <= lenX; i++) {
        free(S[i]);
    }
    free(S);
    free(X); 
    free(Y); 

    return 0;
}
