#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

typedef struct {
    char* X;
    char* Y;
    int** S;
    int lenX;
    int lenY;
    int startCol;
    int endCol;
    int row; 
} ThreadData;

void* calculate_column(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    char* X = data->X;
    char* Y = data->Y;
    int** S = data->S;

    for (int j = data->startCol; j <= data->endCol; j++) {
        int match = S[data->row - 1][j - 1] + ((X[data->row - 1] == Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
        int del = S[data->row - 1][j] + GAP_PENALTY;
        int insert = S[data->row][j - 1] + GAP_PENALTY;
        S[data->row][j] = fmax(fmax(match, del), insert);
    }
    return NULL;
}

void calculate_similarity_matrix_parallel(char* X, char* Y, int lenX, int lenY, int** S) {
    for (int i = 0; i <= lenX; i++) {
        S[i][0] = i * GAP_PENALTY;
    }
    for (int j = 0; j <= lenY; j++) {
        S[0][j] = j * GAP_PENALTY;
    }

    int num_threads = 8;  
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    for (int i = 1; i <= lenX; i++) {
        int colsPerThread = (lenY + num_threads - 1) / num_threads; 
        int startCol; 

        for (int t = 0; t < num_threads; t++) {
            startCol = t * colsPerThread + 1; 
            int endCol = fmin((t + 1) * colsPerThread, lenY);
            if (startCol <= lenY) { 
                thread_data[t] = (ThreadData){X, Y, S, lenX, lenY, startCol, endCol, i};
                pthread_create(&threads[t], NULL, calculate_column, (void*)&thread_data[t]);
            }
        }
        for (int t = 0; t < num_threads; t++) {
            if (startCol <= lenY) { 
                pthread_join(threads[t], NULL);
            }
        }
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

    printf("Optimal Alignment:\n");
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
        fprintf(stderr, "Error: Unable to open file %s\n", filename);
        exit(1);
    }
    // Determine file size
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    printf("File size of %s: %d\n", filename, *length);
    rewind(file);

    *sequence = (char*)malloc((*length + 1) * sizeof(char));
    if (*sequence == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
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

    // Optional: print the S matrix
    // print_matrix(lenX, lenY, S);
    traceback(S, X, Y, lenX, lenY);
    
    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6; 
    printf("Execution time (paralel): %f seconds\n", time_spent);
    
    for (int i = 0; i <= lenX; i++) {
        free(S[i]);
    }
    free(S);
    free(X);
    free(Y);

    return 0;
}