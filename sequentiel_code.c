#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2
#define MAX_SIZE 53640 


void calculate_similarity_matrix(char* X, char* Y, int lenX, int lenY, int** S);
void traceback(int** S, char* X, char* Y, int lenX, int lenY);
void print_matrix(int lenX, int lenY, int** S); 

void calculate_similarity_matrix(char* X, char* Y, int lenX, int lenY, int** S) {
    for (int i = 0; i <= lenX; i++) {
        S[i][0] = i * GAP_PENALTY; 
    }
    for (int j = 0; j <= lenY; j++) {
        S[0][j] = j * GAP_PENALTY; 
    }

    for (int i = 1; i <= lenX; i++) {
        for (int j = 1; j <= lenY; j++) {
            int match = S[i - 1][j - 1] + ((X[i - 1] == Y[j - 1]) ? MATCH_SCORE : MISMATCH_SCORE);
            int del = S[i - 1][j] + GAP_PENALTY;
            int insert = S[i][j - 1] + GAP_PENALTY;
            S[i][j] = fmax(fmax(match, del), insert);
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
            aligned_X[index] = X[i - 1]; // Match ou mismatch
            aligned_Y[index] = Y[j - 1];
            i--;
            j--;
        } else if (i > 0 && S[i][j] == S[i - 1][j] + GAP_PENALTY) {
            aligned_X[index] = X[i - 1]; // Suppression dans Y
            aligned_Y[index] = '-';     // Gap dans Y
            i--;
        } else {
            aligned_X[index] = '-';     // Gap dans X
            aligned_Y[index] = Y[j - 1]; // Insertion dans X
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

void read_sequence_from_file(const char* filename, char* sequence, int* length) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        exit(1);
    }

    *length = 0;
    while ((*length < MAX_SIZE) && (sequence[*length] = fgetc(file)) != EOF) {
        (*length)++;
    }
    sequence[*length] = '\0'; 
    fclose(file);
}

int main() {
    // char X[] = "ATA";  
    // char Y[] = "AGTTA"; 
    char X[MAX_SIZE];
    char Y[MAX_SIZE];
    int lenX, lenY; 
    read_sequence_from_file("X.txt", X, &lenX);
    read_sequence_from_file("Y.txt", Y, &lenY);
    
    // int lenX = sizeof(X) / sizeof(X[0]) - 1; 
    // int lenY = sizeof(Y) / sizeof(Y[0]) - 1; 

    // Déclaration de la matrice de similarité
    int** S = (int**)malloc((lenX + 1) * sizeof(int*));
    for (int i = 0; i <= lenX; i++) {
        S[i] = (int*)malloc((lenY + 1) * sizeof(int));
    }

     clock_t start = clock();
    

    calculate_similarity_matrix(X, Y, lenX, lenY, S);
    printf("Matrice de Similarité :\n");
    // print_matrix(lenX, lenY, S); 

    traceback(S, X, Y, lenX, lenY);
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Temps d'exécution : %f secondes\n", time_spent);


    for (int i = 0; i <= lenX; i++) {
        free(S[i]);
    }
    free(S);

    return 0;
}
