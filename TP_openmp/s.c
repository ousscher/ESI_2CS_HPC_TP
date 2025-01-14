#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define N 250

void print_matrix(float matrix[N][N], int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)  
             printf("%f " , matrix[i][j]);

        printf("\n");
     }
}


void gaussian(float a[N][N], int size)
{
    float l[N];
    for (int i = 0; i < size; i++)
    {
        for (int j = i; j < size; j++)  
        {
            if (i!=j) l[j] = a[j][i] / a[i][i];

            for (int k = 0; k < size; k++)
            { 
                if (j==i) a[j][k] = a[i][k];
                else {
                     if (k < j) a[j][k] = 0;
                     else a[j][k] = a[j][k] - l[j] * a[i][k];
                }       
            }    
        }
    }
}


void random_fill(float matrix[N][N], int size)
{

   srand(time(0));
   for (int i = 0; i < size; i++)
   {
       for (int j = 0; j < size; j++)
       {
           matrix[i][j] = ((rand()%20)+1);
       }
    }
}

int main(void)
{
    int size = N;
    float a[N][N];
    srand(time(0));
    random_fill(a, size);     
    printf("***A***\n");
    print_matrix(a, size);
    clock_t start_t=clock(); //Début_temps
    gaussian(a, size);
    clock_t end_t=clock(); //Fin_temps
    double runing_t=(double)(end_t-start_t)/CLOCKS_PER_SEC;
    printf("***U***\n");
    print_matrix(a, size);
    printf("\nOrdre de la matrice = %d \n " , N);
    printf("\nLe temps séquentiel pour le calcul de la décomposition Gaussienne de la matrice = %f s.\n " , runing_t);
    return 0;
}
