#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROOT 0
#define CHUNK_SIZE 10
#define MATRIX_SIZE 100
#define REQUEST_TAG 999999 // lorsque le processus envoie une requête initiale pour recevoir un chunk
#define FINISH_TAG 1000000

void master_process(int chunk_size, int num_process);
void slave_process(int rank, int chunksize);
void init_matrix(int *matrix, int size);
void print_matrix(int *matrix, int chunk_size, int num_chunks);

int main()
{
    int rank, num_process;
    int chunksize = CHUNK_SIZE;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Processus maître
    if (rank == ROOT)
    {
        master_process(chunksize, num_process);
    }
    // Processus esclaves
    else
    {
        slave_process(rank, chunksize);
    }

    MPI_Finalize();
    return 0;
}

void master_process(int chunksize, int num_process)
{
    int num_chunks = MATRIX_SIZE / chunksize;
    int data[MATRIX_SIZE];
    int chunk[chunksize];
    MPI_Status status;

    // Variable qui suit le nombre de chunks de données distribués
    int chunks_sent = 0;
    // Variable utilisée pour suivre le nombre de chunks traités et retournés par les esclaves
    int chunks_received = 0;
    // Compteur pour le nombre d'esclaves actifs
    int active_slaves = num_process - 1;

    printf("Initialisation des données : \n");
    init_matrix(data, MATRIX_SIZE);
    print_matrix(data, chunksize, num_chunks);

    // Tant que tous les chunks n'ont pas été reçus ou que certains processus esclaves sont encore actifs
    while (chunks_received < num_chunks || active_slaves > 0)
    {
        MPI_Recv(chunk, chunksize, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Stocker les données traitées des esclaves
        if (status.MPI_TAG != REQUEST_TAG)
        {
            for (int i = 0; i < chunksize; i++)
            {
                data[status.MPI_TAG * chunksize + i] = chunk[i];
            };
            chunks_received++;
        }

        // Vérifier s'il reste des chunks de données à envoyer
        if (chunks_sent < num_chunks)
        {
            MPI_Send(&data[chunks_sent * chunksize], chunksize, MPI_INT, status.MPI_SOURCE, chunks_sent, MPI_COMM_WORLD);
            chunks_sent++;
        }
        else
        {
            MPI_Send(NULL, 0, MPI_INT, status.MPI_SOURCE, FINISH_TAG, MPI_COMM_WORLD);
            active_slaves--;
        }
    }

    printf("Données modifiées : \n");
    print_matrix(data, chunksize, num_chunks);
}


void slave_process(int rank, int chunksize)
{

    int chunk[chunksize];
    MPI_Status status;

    // Requête initiale pour demander des données
    MPI_Send(NULL, 0, MPI_INT, ROOT, REQUEST_TAG, MPI_COMM_WORLD);

    while (1)
    {
        // Recevoir des données ou un signal de terminaison
        MPI_Recv(chunk, chunksize, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == FINISH_TAG)
            break;

        // Traiter les données
        for (int i = 0; i < chunksize; ++i)
        {
            chunk[i] = chunk[i] * chunk[i];
        }
        // Envoyer les données traitées au maître, ce qui constitue également une nouvelle requête de données
        MPI_Send(chunk, chunksize, MPI_INT, ROOT, status.MPI_TAG, MPI_COMM_WORLD);
    }
}

void init_matrix(int *matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        matrix[i] = i;
    }
}
void print_matrix(int *matrix, int chunk_size, int num_chunks)
{
    int matrix_size = chunk_size * num_chunks;
    int columns = chunk_size; // Columns per row (corresponding to chunk size)

    printf("\nMatrix Display:\n");
    printf("----------------------------\n");

    printf("     ");
    for (int col = 0; col < columns; col++)
    {
        printf("%5d ", col);
    }
    printf("\n");

    printf("     ");
    for (int col = 0; col < columns; col++)
    {
        printf("------");
    }
    printf("\n");

    // Print matrix as rows and columns
    for (int i = 0; i < matrix_size; i++)
    {
        if (i % columns == 0) // Row header and new line
        {
            printf("Row %2d | ", i / columns);
        }
        printf("%5d ", matrix[i]);

        if ((i + 1) % columns == 0) 
        {
            printf("\n");
        }
    }

    printf("----------------------------\n");
}
