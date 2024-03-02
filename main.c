#include "headers.h"

int main(int argc, char const *argv[]) {
    //  Initializare variabile folosite pentru citire
    int nrInputs, outputIdx;
    FILE *read = NULL;
    FILE *write = NULL;

    //  Initializare arbore
    TArb arbore = AlocaNod();
    if (!arbore) {
        printf("Malloc failed");
        return 0;
    }

    //  Deschiderea fisierelor
    read = fopen(argv[1], "r");
    write = fopen(argv[2], "w");

    //  Citeste numarul de variabile si indexul portii care este output
    fscanf(read, "%d", &nrInputs);
    fscanf(read, "%d", &outputIdx);

    //  Numarul de porti
    int nrPorti = outputIdx - nrInputs + 1;

    //  Initializare matrice care retine datele din fisier despre porti
    int **portiInfo = malloc(nrPorti * sizeof(int *));
    if (!portiInfo) {
        printf("Malloc failed");
        return 0;
    }
    for (int i = 0; i < nrPorti; i++) {
        portiInfo[i] = calloc((MAXINPUT + 2), sizeof(int));
        if (!portiInfo[i]) {
            printf("Calloc failed");
            for (int j = 0; j < i; i++)
                free(portiInfo[j]);
            free(portiInfo);
            return 0;
        }
    }

    //  Se citesc si retin informatii despre porti intr-o matrice
    for (int i = 0; i <= outputIdx - nrInputs; i++) {
        char str[200];
        fgets(str, 200, read);

        char *p = strtok(str, " \n");
        int j = 2;
        while (p) {
            if (atoi(p) == 0) {
                //  Verificare tip de poarta
                if (strcmp(p, "AND") == 0)
                    portiInfo[i][0] = 1;
                else if (strcmp(p, "OR") == 0)
                    portiInfo[i][0] = 2;
                else
                    portiInfo[i][0] = 3;
            } else {
                portiInfo[i][j++] = atoi(p);
            }

            p = strtok(NULL, " \n");
        }
        portiInfo[i][1] = portiInfo[i][j-1];
        portiInfo[i][j-1] = 0; 
    }

    //  Contruire arbore
    ContruireArbore(arbore, portiInfo, nrPorti, nrInputs, outputIdx);
    InitializareParinti(arbore, NULL);

    //  Eliminare porti NOT
    if (arbore->tipPoarta == 3) {
        EliminarePortiNOT(arbore->fii[0], arbore, 1);
        arbore = arbore->fii[0];
        outputIdx = arbore->indice;
    } else {
        EliminarePortiNOT(arbore, NULL, 0);
    }

    //  Se elimina partile de tip AND/OR care au parintele de acelasi tip ca ele
    EliminaPortileInutile(arbore, NULL, 1);
    EliminaPortileInutile(arbore, NULL, 2);

    //  Reducem arborele la o forma CNFSAT
    ReduceLaCNFSAT(arbore, NULL, outputIdx);

    //  Afisare in fisier de output
    fprintf(write, "p cnf %d ", nrInputs);

    //  Daca radacina arborelui este o poarta OR sau un input, exista doar o clauza
    if (arbore->tip == 1) {
        fprintf(write, "1\n");
        fprintf(write, "%d 0\n", arbore->indice);
    } else if (arbore->tipPoarta == 2) {
        fprintf(write, "1\n");
        for (int i = 0; arbore->fii[i]; i++)
            fprintf(write, "%d ", arbore->fii[i]->indice);
        fprintf(write, "0\n");
    } else {
        //  Numaram fiii radacinii, adica numarul de clauze
        int nrFii = 0;
        for (int i = 0; arbore->fii[i] != 0; i++)
            nrFii++;
        fprintf(write, "%d\n", nrFii);
        AfisareClauze(arbore, write);
    }

    //  Dezalocare matrice
    for (int i = 0; i < nrPorti; i++)
        free(portiInfo[i]);
    free(portiInfo);

    DistrArb(&arbore);
    fclose(read);
    fclose(write);
    return 0;
}