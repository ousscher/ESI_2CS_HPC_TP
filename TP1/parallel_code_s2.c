
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h> 


#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2
#define NUM_THREADS 2

typedef struct {
    int **S;
    char *X;
    char *Y;
    int lenX;
    int lenY;
    int antidiagonal_start;
    int antidiagonal_end;
} ThreadData;

pthread_mutex_t **mutex_matrix; 
pthread_cond_t **cond_matrix;  

int max(int a, int b, int c) {
    return fmax(fmax(a, b), c);
}

void calculate_cell(ThreadData* data, int i, int j) {
    int **S = data->S;
    char *X = data->X;
    char *Y = data->Y;

    pthread_mutex_lock(&mutex_matrix[i-1][j-1]);
    while (S[i-1][j-1] == INT_MIN) pthread_cond_wait(&cond_matrix[i-1][j-1], &mutex_matrix[i-1][j-1]);
    pthread_mutex_unlock(&mutex_matrix[i-1][j-1]);

    pthread_mutex_lock(&mutex_matrix[i-1][j]);
    while (S[i-1][j] == INT_MIN) pthread_cond_wait(&cond_matrix[i-1][j], &mutex_matrix[i-1][j]);
    pthread_mutex_unlock(&mutex_matrix[i-1][j]);

    pthread_mutex_lock(&mutex_matrix[i][j-1]);
    while (S[i][j-1] == INT_MIN) pthread_cond_wait(&cond_matrix[i][j-1], &mutex_matrix[i][j-1]);
    pthread_mutex_unlock(&mutex_matrix[i][j-1]);

    int match = S[i - 1][j - 1] + ((X[i - 1] == Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
    int del = S[i - 1][j] + GAP_PENALTY;
    int insert = S[i][j - 1] + GAP_PENALTY;
    S[i][j] = max(match, del, insert);

    pthread_mutex_lock(&mutex_matrix[i][j]);
    pthread_cond_broadcast(&cond_matrix[i][j]);
    pthread_mutex_unlock(&mutex_matrix[i][j]);
}

void* calculate_antidiagonals(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    for (int k = data->antidiagonal_start; k <= data->antidiagonal_end; k++) {
        for (int i = 1; i <= data->lenX; i++) {
            int j = k - i;
            if (j >= 1 && j <= data->lenY) {
                calculate_cell(data, i, j);
            }
        }
    }
    return NULL;
}

void calculate_similarity_matrix_parallel(char* X, char* Y, int lenX, int lenY, int** S) {
    // Initialize matrix boundaries with gap penalties
    for (int i = 0; i <= lenX; i++) S[i][0] = i * GAP_PENALTY;
    for (int j = 0; j <= lenY; j++) S[0][j] = j * GAP_PENALTY;

    // Initialize mutexes and condition variables for each cell
    mutex_matrix = malloc((lenX + 1) * sizeof(pthread_mutex_t*));
    cond_matrix = malloc((lenX + 1) * sizeof(pthread_cond_t*));
    for (int i = 0; i <= lenX; i++) {
        mutex_matrix[i] = malloc((lenY + 1) * sizeof(pthread_mutex_t));
        cond_matrix[i] = malloc((lenY + 1) * sizeof(pthread_cond_t));
        for (int j = 0; j <= lenY; j++) {
            pthread_mutex_init(&mutex_matrix[i][j], NULL);
            pthread_cond_init(&cond_matrix[i][j], NULL);
            if (i > 0 && j > 0) S[i][j] = INT_MIN; 
        }
    }

    int max_antidiagonal = lenX + lenY;

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    int antidiags_per_thread = max_antidiagonal / NUM_THREADS;
    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * antidiags_per_thread + 2;
        int end = (t == NUM_THREADS - 1) ? max_antidiagonal : start + antidiags_per_thread - 1;
        thread_data[t] = (ThreadData){S, X, Y, lenX, lenY, start, end};
        pthread_create(&threads[t], NULL, calculate_antidiagonals, &thread_data[t]);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // Clean up 
    for (int i = 0; i <= lenX; i++) {
        for (int j = 0; j <= lenY; j++) {
            pthread_mutex_destroy(&mutex_matrix[i][j]);
            pthread_cond_destroy(&cond_matrix[i][j]);
        }
        free(mutex_matrix[i]);
        free(cond_matrix[i]);
    }
    free(mutex_matrix);
    free(cond_matrix);
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

    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6; 
    // print_matrix(lenX, lenY, S);
    traceback(S, X, Y, lenX, lenY);

    printf("Temps d'exécution : %.6f secondes\n", time_spent);

    for (int i = 0; i <= lenX; i++) {
        free(S[i]);
    }
    free(S);
    free(X);
    free(Y);
    return 0;
}
