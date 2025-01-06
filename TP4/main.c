#include <mpi.h> 
#include <stdio.h>
#include <stdlib.h>

#define TAILLE_BLOC 10
#define TAILLE_MATRICE 100
#define TAG_REQUETE_INITIALE 999999 // lorsque le processus envoie une requête initiale pour recevoir un bloc
#define TAG_TERMINATION 1000000

void processus_maitre(int taille_bloc, int nb_processus);
void processus_esclave(int rang, int taille_bloc);
void initialiser_matrice(int *matrice, int taille);
void afficher_matrice(int *matrice, int taille_bloc, int nb_blocs);

int main() {
    int rang, nb_processus;
    int taille_bloc = TAILLE_BLOC;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_processus);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);

    // Processus maître
    if (rang == 0) {
        processus_maitre(taille_bloc, nb_processus);
    } else {
        processus_esclave(rang, taille_bloc);
    }

    MPI_Finalize();
    return 0;
}

void processus_maitre(int taille_bloc, int nb_processus) {
    int nb_blocs = TAILLE_MATRICE / taille_bloc;
    int matrice[TAILLE_MATRICE];
    int bloc[taille_bloc];
    MPI_Status statut;

    int blocs_envoyes = 0; // le nombre de blocs de données distribués
    int blocs_recus = 0; // le nombre de blocs traités et retournés par les esclaves
    int esclaves_actifs = nb_processus - 1; // le nombre d'esclaves actifs

    printf("Initialisation des données : \n");
    initialiser_matrice(matrice, TAILLE_MATRICE);
    afficher_matrice(matrice, taille_bloc, nb_blocs);

    // Tant que tous les blocs n'ont pas été reçus ou que certains processus esclaves sont encore actifs
    while (blocs_recus < nb_blocs || esclaves_actifs > 0) {
        MPI_Recv(bloc, taille_bloc, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &statut);

        // Stocker les données traitées des esclaves
        if (statut.MPI_TAG != TAG_REQUETE_INITIALE) {
            for (int i = 0; i < taille_bloc; i++) {
                matrice[statut.MPI_TAG * taille_bloc + i] = bloc[i];
            }
            blocs_recus++;
        }

        // Vérifier s'il reste des blocs de données à envoyer
        if (blocs_envoyes < nb_blocs) {
            MPI_Send(&matrice[blocs_envoyes * taille_bloc], taille_bloc, MPI_INT, statut.MPI_SOURCE, blocs_envoyes, MPI_COMM_WORLD);
            blocs_envoyes++;
        } else {
            MPI_Send(NULL, 0, MPI_INT, statut.MPI_SOURCE, TAG_TERMINATION, MPI_COMM_WORLD);
            esclaves_actifs--;
        }
    }

    printf("Données modifiées : \n");
    afficher_matrice(matrice, taille_bloc, nb_blocs);
}

void processus_esclave(int rang, int taille_bloc) {
    int bloc[taille_bloc];
    MPI_Status statut;

    // Requête initiale pour demander des données
    MPI_Send(NULL, 0, MPI_INT, 0, TAG_REQUETE_INITIALE, MPI_COMM_WORLD);

    while (1) {
        // Recevoir des données ou un signal de terminaison
        MPI_Recv(bloc, taille_bloc, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &statut);

        if (statut.MPI_TAG == TAG_TERMINATION)
            break;

        // Traiter les données
        for (int i = 0; i < taille_bloc; ++i) {
            bloc[i] = bloc[i] * bloc[i];
        }

        // Envoyer les données traitées au maître, ce qui constitue également une nouvelle requête de données
        MPI_Send(bloc, taille_bloc, MPI_INT, 0, statut.MPI_TAG, MPI_COMM_WORLD);
    }
}

void initialiser_matrice(int *matrice, int taille) {
    for (int i = 0; i < taille; i++) {
        matrice[i] = i;
    }
}

void afficher_matrice(int *matrice, int taille_bloc, int nb_blocs) {
    int taille_totale = taille_bloc * nb_blocs;
    int colonnes_par_bloc = taille_bloc;

    printf("\nAffichage de la matrice :\n");
    printf("----------------------------\n");

    printf("     ");
    for (int col = 0; col < colonnes_par_bloc; col++) {
        printf("%5d ", col);
    }
    printf("\n");

    printf("     ");
    for (int col = 0; col < colonnes_par_bloc; col++) {
        printf("------");
    }
    printf("\n");

    for (int i = 0; i < taille_totale; i++) {
        if (i % colonnes_par_bloc == 0) {
            printf("Ligne %2d | ", i / colonnes_par_bloc);
        }
        printf("%5d ", matrice[i]);

        if ((i + 1) % colonnes_par_bloc == 0) {
            printf("\n");
        }
    }

    printf("----------------------------\n");
}