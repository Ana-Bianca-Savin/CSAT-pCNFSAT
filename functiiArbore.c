#include "headers.h"

//  Alocarea unui nod
TArb AlocaNod() {
    TArb nod = malloc(sizeof(TNod));
    if (!nod) {
        printf("Malloc failed");
        return NULL;
    }

    nod->fii = calloc(MAXINPUT, sizeof(TNod *));
    if (!nod->fii) {
        free(nod);
        printf("Calloc failed");
        return NULL;
    }    

    return nod;
}

void ContruireArbore(TArb a, int **portiInfo, int nrPorti, int nrInputs, int index) {
    a->indice = index;

    if (index <= nrInputs) {
        //  Nodul curent reprezinta o variabila, nu o poarta
        a->tip = 1;
        a->tipPoarta = -1;
    } else {
        //  Nodul curent reprezinta o poarta
        //  Este cautata poarta in matrice, ea se afla pe coloana 1
        for (int i = 0; i < nrPorti; i++)
            if (portiInfo[i][1] == index) {
                //  Initializarea nodului
                a->tip = 2;
                a->tipPoarta = portiInfo[i][0];

                //  Adaugarea nodurilor copil in vectorul de fii
                int k = 0;
                while (a->fii[k] != NULL);

                for (int j = 2; portiInfo[i][j] != 0; j++) {
                    a->fii[k] = AlocaNod();
                    ContruireArbore(a->fii[k], portiInfo, nrPorti, nrInputs, portiInfo[i][j]);
                    k++;
                }
            }
    }
}

void InitializareParinti(TArb a, TArb parinte) {
    if (a == NULL)
        return;

    a->parinte = parinte;

    int k = 0;
    while (a->fii[k] != NULL) {
        InitializareParinti(a->fii[k], a);
        k++;
    }
}

void EliminarePortiNOT(TArb a, TArb parinte, int underNegation) {
    if (a == NULL)
        return;

    if (a->tipPoarta == 3) {
        //  Folosim while, ca sa fie eliminate si portiile NOT consecutive
        while (a->tipPoarta == 3) {
            if (underNegation == 1)
                underNegation = 0;
            else
                underNegation = 1;

            //  Stergerea portii
            for (int i = 0; parinte->fii[i] != 0; i++)
                if (a == parinte->fii[i]) {
                    //  Pe pozitia poartii NOT, va fi adaugat unicul fiu
                    parinte->fii[i] = a->fii[0];
                    break;
                }

            a = a->fii[0];
        }
    }
    if (underNegation == 1) {
        //  Neaga informatia curenta
        if (a->tip == 1) {
            //  Daca este variabila, se neaga indexul
            a->indice = -a->indice;
        } else if (a->tipPoarta == 1) {
            //  Daca poarta este AND, devine OR
            a->tipPoarta = 2;
        } else if (a->tipPoarta == 2) {
            //  Daca poarta este OR, devine AND
            a->tipPoarta = 1;
        }
    }

    int k = 0;
    while (a->fii[k] != NULL) {
        EliminarePortiNOT(a->fii[k], a, underNegation);
        k++;
    }
}

void EliminaPortileInutile(TArb a, TArb parinte, int tipPoarta) {
    if (a == NULL)
        return;

    //  Daca poarta curenta si poarta parinte au acelasi tip (AND/OR),
    //  poarta parinte va adopta copiii portii copil, ce va fi stearsa
    if (parinte != NULL) {
        if (a->tipPoarta == parinte->tipPoarta && parinte->tipPoarta == tipPoarta) {
            //  Verificam daca parintele are MAXINPUT de fii deja
            int cnt, nrFii;
            for (cnt = 0; parinte->fii[cnt] != 0; cnt++);
            for (nrFii = 0; a->fii[nrFii] != NULL; nrFii++);

            if (cnt + nrFii >= MAXINPUT - 1) {
                //  Alocam spatiu pentru fiii ce vor fi adaugati
                parinte->fii = realloc(parinte->fii, (cnt + nrFii + 1) * sizeof(TNod *));

                //  Initializare locuri libere cu NULL
                for (int i = cnt; i <= cnt + nrFii; i++)
                    parinte->fii[i] = NULL;
            }

            //  Se cauta nodul curent in lista parintelui
            for (int i = 0; parinte->fii[i] != 0; i++)
                if (a == parinte->fii[i]) {
                    int j = 0;
                    for (j = i; parinte->fii[j] != NULL; j++)
                        parinte->fii[j] = parinte->fii[j + 1];

                    j--;
                    //  Mutarea copiilor
                    for (int k = 0; a->fii[k] != NULL; k++)
                        parinte->fii[j++] = a->fii[k];

                    break;
                }

            a = parinte;
            parinte = parinte->parinte;
        }
    }

    int k = 0;
    while (a->fii[k] != NULL) {
        EliminaPortileInutile(a->fii[k], a, tipPoarta);
        k++;
    }
}

void VerifPereche(TArb **perechi) {
    //  O pereche nu poate fi formata drintr-un singur input
    if (perechi[0][1] == NULL)
        return;

    //  Cautam ultima linie libera din matrice
    int j = 0;
    for (j = 1; perechi[j][0] != NULL; j++) {
        //  Verificam daca perechea exista deja
        int all = 1;
        for (int i = 0; perechi[0][i] != NULL && perechi[j][i] != NULL; i++)
            if (perechi[j][i]->indice != perechi[0][i]->indice)
                all = 0;

        if (all == 1)
            return;
    }

    //  Adaugam perechea gasita pe linie
    for (int k = 0; perechi[0][k] != 0; k++) {
        perechi[j][k] = perechi[0][k];
    }
}

void DistribuireOR(TArb a, TArb parinte, TArb mareParinte, TArb **perechi, int indiceFiuParinteMare , int numarFii) {
    if (indiceFiuParinteMare + 1 == numarFii) {
        VerifPereche(perechi);
        return;
    }

    //  Poarta de tip OR
    if (a->tipPoarta == 2) {

        //  Se parcurg fiii
        for (int k = 0; a->fii[k] != NULL; k++) {
            //  Fiii pot fi doar AND sau INPUT
            if (a->fii[k]->tip == 1) {
                //  Daca fiul este input

                //  Parcurgem perechea 0, pentru a gasi ultimul indice liber
                int i = 0;
                while (perechi[0][i] != 0)
                    i++;

                //  Adaug inputul la pereche
                perechi[0][i] = a->fii[k];

                //  Apel recursiv
                DistribuireOR(mareParinte->fii[indiceFiuParinteMare + 1], mareParinte, mareParinte, perechi, indiceFiuParinteMare + 1, numarFii);

                //  Eliminam inputul anterior de la perechea curenta
                perechi[0][i] = NULL;
            } else if (a->fii[k]->tipPoarta == 1) {
                //  Daca este poarta AND
                TArb AND = a->fii[k];

                //  Se parcurg copiii
                for (int j = 0; AND->fii[j] != 0; j++) {
                    //  Parcurgem perechea 0, pentru a gasi ultimul indice liber
                    int i = 0;
                    while (perechi[0][i] != NULL)
                        i++;

                    //  Copiii pot fi OR sau input
                    if (AND->fii[j]->tip == 1) {
                        //  Copilul este input

                        //  Adaug inputul la pereche
                        perechi[0][i] = AND->fii[j];

                        //  Apel recursiv
                        DistribuireOR(mareParinte->fii[indiceFiuParinteMare + 1], mareParinte, mareParinte, perechi, indiceFiuParinteMare + 1, numarFii);

                        //  Eliminam inputul anterior de la perechea curenta
                        perechi[0][i] = NULL;
                        
                    } else if (AND->fii[j]->tipPoarta == 2) {
                        //  Copilul este poarta OR
                        int cnt = 0;

                        //  Adaug copiii lui OR la pereche (ESTE GARANTAT CA SUNT INPUT)
                        for (cnt = 0; AND->fii[j]->fii[cnt] != 0; cnt++) {
                            perechi[0][i] = AND->fii[j]->fii[cnt];
                            i++;
                        }

                        //  Apel recursiv pentru urmatorul fiu al parintelui mare
                        DistribuireOR(mareParinte->fii[indiceFiuParinteMare + 1], mareParinte, mareParinte, perechi, indiceFiuParinteMare + 1, numarFii);

                        //  Scoatem din perechea curenta fii portii OR adaugati inainte
                        for (int j = i - 1; cnt > 0; j--, cnt--) {
                            perechi[0][j] = NULL;
                        }
                    }
                }
            }
        }
    }
}

void DistribuireORUnSingurFiu(TArb a, TArb **perechi) {
    //  Poarta curenta este OR
    if (a->tipPoarta == 2) {
        //  Parcurgerea copiilor
        for (int k = 0; a->fii[k] != NULL; k++) {
            if (a->fii[k]->tip == 1) {
                //  Copilul este input

                //  Parcurgem perechea 0, pentru a gasi ultimul indice liber
                int i = 0;
                while (perechi[0][i] != 0)
                    i++;

                //  Adaug inputul la pereche
                perechi[0][i] = a->fii[k];

                //  Verificare pereche
                VerifPereche(perechi);

                //  Eliminam inputul anterior de la perechea curenta
                perechi[0][i] = NULL;
            } else if (a->fii[k]->tipPoarta == 1) {
                //  Copilul este poarta de tip AND
                TArb AND = a->fii[k];

                //  Se parcurg copiii
                for (int j = 0; AND->fii[j] != 0; j++) {
                    //  Parcurgem perechea 0, pentru a gasi ultimul indice liber
                    int i = 0;
                    while (perechi[0][i] != NULL)
                        i++;

                    //  Copiii pot fi OR sau input
                    if (AND->fii[j]->tip == 1) {
                        //  Copilul este input

                        //  Adaug inputul la pereche
                        perechi[0][i] = AND->fii[j];

                        //  Verificare pereche
                        VerifPereche(perechi);

                        //  Eliminam inputul anterior de la perechea curenta
                        perechi[0][i] = NULL;
                        
                    } else if (AND->fii[j]->tipPoarta == 2) {
                        //  Copilul este poarta OR
                        int cnt = 0;

                        //  Adaug copiii lui OR la pereche ESTE GARANTAT CA SUNT INPUT
                        for (cnt = 0; AND->fii[j]->fii[cnt] != 0; cnt++) {
                            perechi[0][i] = AND->fii[j]->fii[cnt];
                            i++;
                        }

                        //  Verificare pereche
                        VerifPereche(perechi);

                        //  Scoatem din perechea curenta fii portii OR adaugati inainte
                        for (int j = i - 1; cnt > 0; j--, cnt--) {
                            perechi[0][j] = NULL;
                        }
                    }
                }
            }
        }
    }
}

void ReduceLaCNFSAT(TArb a, TArb parinte, int outputIdx) {
    if (a == NULL)
        return;

    //  Apelul recursiv mai intai, ca parcurgerea sa se faca invers
    for (int i = 0; a->fii[i] != NULL; i++) {
        TArb temp = a->fii[i];

        ReduceLaCNFSAT(a->fii[i], a, outputIdx);

        //  a este diferit de valoarea sa initiala daca s-a unit cu o poarta AND parinte
        if (a->fii[i] != temp)
            i--;
    }

    //  Daca poarta curenta este OR
    if (a->tipPoarta == 2) {
        //  Verificam daca poarta OR trebuie distribuita peste AND
        int ok = 1;
        for (int i = 0; a->fii[i] != NULL; i++)
            if (a->fii[i]->tipPoarta == 1)
                ok = 0;

        if (ok == 0) {
            //  Trebuie sa facem distribuirea portii OR

            //  Alocare matrice ce retine perechi
            TArb **perechi = calloc(MAXPAIRS, sizeof(TArb *));
            if (!perechi) {
                printf("Calloc failed");
                return;
            }
            for (int i = 0; i < MAXPAIRS; i++)
                perechi[i] = calloc(MAXPAIRS, sizeof(TArb));

            //  Numaram fiii portii OR
            int nrFii, cnt = 0;
            for (nrFii = 0; a->fii[nrFii] != NULL; nrFii++) {
                //  Daca exista fii care sunt variabile, acestia sunt stersi si pusi in perechea 0
                if (a->fii[nrFii]->tip == 1) {
                    perechi[0][cnt++] = a->fii[nrFii];

                    for (int j = nrFii; a->fii[j] != NULL; j++)
                        a->fii[j] = a->fii[j + 1];
                    nrFii--;
                }
            }

            //  Distribuirea portii
            if (nrFii == 1) {
                DistribuireORUnSingurFiu(a, perechi);
            } else {
                DistribuireOR(a, parinte, a, perechi, 0, nrFii);
            }

            //  Poarta curenta devine de tip AND
            a->tipPoarta = 1;

            //  Se sterg toti copiii portii vechi
            for (int i = 0; a->fii[i] != 0; i++) {
                a->fii[i] = NULL;
            }

            //  Se creeaza noi porti OR pentru fiecare pereche
            //  Ele devin fiii portii curente
            cnt = 0;
            for (int i = 1; perechi[i][0] != 0; i++) {
                TArb aux = AlocaNod();
                aux->indice = outputIdx + i;
                aux->tip = 2;
                aux->tipPoarta = 2;

                for (int j = 0; perechi[i][j] != 0; j++) {
                    aux->fii[j] = AlocaNod();
                    aux->fii[j]->indice = perechi[i][j]->indice;
                    aux->fii[j]->tip = perechi[i][j]->tip;
                    aux->fii[j]->tipPoarta = perechi[i][j]->tipPoarta;
                }

                a->fii[cnt++] = aux;
            }

            //  Se corecteaza arborele prin stergerea portilor AND cu parinti AND
            EliminaPortileInutile(a, parinte, 1);

            //  Dezalocare matrice
            for (int i = 0; i < MAXPAIRS; i++)
                free(perechi[i]);
            free(perechi);
        }
    }
}

void AfisareClauze(TArb a, FILE *write) {
    for (int i = 0; a->fii[i] != 0; i++) {
        if (a->fii[i]->tipPoarta == 2)
            for (int j = 0; a->fii[i]->fii[j] != 0; j++)
                fprintf(write, "%d ", a->fii[i]->fii[j]->indice);
        else
            fprintf(write, "%d ", a->fii[i]->indice);
        fprintf(write, "0\n");
    }
}

//  Distrugere auxiliar
void Distruge(TArb a) {
    if (!a)
        return;

    int k = 0;
    while (a->fii[k] != NULL) {
        Distruge(a->fii[k]);
        k++;
    }
    free(a->fii);
    free(a);
}

//  Distrugere arbore
void DistrArb(TArb *a) {
    if (!(*a))
        return;
    Distruge(*a);
    *a = NULL;
}
