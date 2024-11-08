#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>


#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

typedef struct {
    int** S;        // Similarity matrix
    char* X;       // First sequence
    char* Y;       // Second sequence
    int lenX;      // Length of first sequence
    int lenY;      // Length of second sequence
    int start;     // Starting index for row or column
    int end;       // Ending index for row or column
} ThreadData;

void* calculate_rows(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int i = data->start; i <= data->end; i++) {
        for (int j = 1; j <= data->lenY; j++) {
            int match = data->S[i - 1][j - 1] + ((data->X[i - 1] == data->Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
            int del = data->S[i - 1][j] + GAP_PENALTY;
            int insert = data->S[i][j - 1] + GAP_PENALTY;
            data->S[i][j] = fmax(fmax(match, del), insert);
        }
    }
    return NULL;
}

void* calculate_cols(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    for (int j = data->start; j <= data->end; j++) {
        for (int i = 1; i <= data->lenX; i++) {
            int match = data->S[i - 1][j - 1] + ((data->X[i - 1] == data->Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
            int del = data->S[i - 1][j] + GAP_PENALTY;
            int insert = data->S[i][j - 1] + GAP_PENALTY;
            data->S[i][j] = fmax(fmax(match, del), insert);
        }
    }
    return NULL;
}

void calculate_similarity_matrix_parallel(char* X, char* Y, int lenX, int lenY, int** S) {
    // Initialisation des bordures de la matrice
    for (int i = 0; i <= lenX; i++) S[i][0] = i * GAP_PENALTY;
    for (int j = 0; j <= lenY; j++) S[0][j] = j * GAP_PENALTY;

    pthread_t t_row, t_col;
    
    // Création des threads pour chaque paire d'indices de la diagonale
    for (int i = 1; i <= lenX; i += 2) {
        ThreadData row_data = {S, X, Y, lenX, lenY, i, fmin(i + 1, lenX)}; // calculate two rows
        ThreadData col_data = {S, X, Y, lenX, lenY, i, fmin(i + 1, lenY)}; // calculate two columns
        
        pthread_create(&t_row, NULL, calculate_rows, &row_data);
        pthread_create(&t_col, NULL, calculate_cols, &col_data);
        
        pthread_join(t_row, NULL);
        pthread_join(t_col, NULL);
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

    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6; 

    // print_matrix(lenX, lenY, S);
    traceback(S, X, Y, lenX, lenY);
    printf("Temps d'exécution : %f secondes\n", time_spent);
    for (int i = 0; i <= lenX; i++) {
        free(S[i]);
    }
    free(S);
    free(X);
    free(Y);

    return 0;
}

