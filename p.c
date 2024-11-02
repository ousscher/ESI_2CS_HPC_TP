#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>

#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2
#define NUM_THREADS 8

typedef struct {
    char* X;
    char* Y;
    int lenX;
    int lenY;
    int** S;
    int start_diag;
    int end_diag;
} ThreadData;

void* calculate_diagonal(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    char* X = data->X;
    char* Y = data->Y;
    int** S = data->S;

    for (int diag = data->start_diag; diag <= data->end_diag; diag++) {
        for (int i = 1; i <= diag - 1; i++) {
            int j = diag - i;  
            if (j < 1 || j > data->lenY || i < 1 || i > data->lenX) continue; 

            int match = S[i - 1][j - 1] + ((X[i - 1] == Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
            int del = S[i - 1][j] + GAP_PENALTY;
            int insert = S[i][j - 1] + GAP_PENALTY;
            S[i][j] = fmax(fmax(match, del), insert);
        }
    }

    return NULL;
}

void calculate_similarity_matrix(char* X, char* Y, int lenX, int lenY, int** S) {
    for (int i = 0; i <= lenX; i++) {
        S[i][0] = i * GAP_PENALTY; 
    }
    for (int j = 0; j <= lenY; j++) {
        S[0][j] = j * GAP_PENALTY; 
    }

    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    int total_diags = lenX + lenY - 1;
    
    int blocks_per_thread[NUM_THREADS] = {1, 2, 3, 4, 5, 6, 7, 8}; // Définir le nombre de blocs que chaque thread va traiter

    for (int thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
        int start_diag = (thread_id == 0) ? 2 : (threadData[thread_id - 1].end_diag + 1);
        int end_diag = start_diag + blocks_per_thread[thread_id] - 1;

        // Assurer que end_diag ne dépasse pas total_diags
        if (end_diag > total_diags) {
            end_diag = total_diags;
        }

        threadData[thread_id].X = X;
        threadData[thread_id].Y = Y;
        threadData[thread_id].lenX = lenX;
        threadData[thread_id].lenY = lenY;
        threadData[thread_id].S = S;
        threadData[thread_id].start_diag = start_diag;
        threadData[thread_id].end_diag = end_diag;

        pthread_create(&threads[thread_id], NULL, calculate_diagonal, (void*)&threadData[thread_id]);
    }

    // Attendre que tous les threads aient terminé
    for (int thread_id = 0; thread_id < NUM_THREADS; thread_id++) {
        pthread_join(threads[thread_id], NULL);
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
    calculate_similarity_matrix(X, Y, lenX, lenY, S);
    gettimeofday(&end, NULL);

    double time_spent = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_usec - start.tv_usec) / 1e6; 
    print_matrix(lenX, lenY, S);
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
