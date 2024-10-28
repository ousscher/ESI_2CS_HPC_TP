#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATCH_SCORE 1
#define MISMATCH_SCORE -1
#define GAP_PENALTY -2

int score(char a, char b) {
    return (a == b) ? MATCH_SCORE : MISMATCH_SCORE;
}

void calculate_similarity_matrix(char* X, char* Y, int lenX, int lenY, int S[lenX+1][lenY+1]) {
    for (int i = 0; i <= lenX; i++) {
        S[i][0] = i * GAP_PENALTY;
    }
    for (int j = 0; j <= lenY; j++) {
        S[0][j] = j * GAP_PENALTY;
    }

    for (int i = 1; i <= lenX; i++) {
        for (int j = 1; j <= lenY; j++) {
            int match = S[i-1][j-1] + score(X[i-1], Y[j-1]);
            int del = S[i-1][j] + GAP_PENALTY;
            int insert = S[i][j-1] + GAP_PENALTY;
            S[i][j] = (match > del) ? ((match > insert) ? match : insert) : ((del > insert) ? del : insert);
        }
    }
}

void print_matrix(int lenX, int lenY, int S[lenX+1][lenY+1]) {
    for (int i = 0; i <= lenX; i++) {
        for (int j = 0; j <= lenY; j++) {
            printf("%3d ", S[i][j]);
        }
        printf("\n");
    }
}

int main() {
    char X[] = "ACGTACGTAC";  
    char Y[] = "ACGTCGTGCA";
    
    int lenX = sizeof(X) / sizeof(X[0]) - 1; 
    int lenY = sizeof(Y) / sizeof(Y[0]) - 1; 

    int S[lenX+1][lenY+1];
    
    clock_t start = clock();
    calculate_similarity_matrix(X, Y, lenX, lenY, S);
    clock_t end = clock();
    
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Matrice de Similarité :\n");
    print_matrix(lenX, lenY, S);
    
    printf("Temps d'exécution : %f secondes\n", time_spent);

    return 0;
}
