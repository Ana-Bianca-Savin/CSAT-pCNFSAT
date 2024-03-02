#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXINPUT 61
#define MAXPAIRS 500

//  Structura pentru arbore
typedef struct nod {
    int indice;
    int tip; // 1 pt input; 2 pt poarta
    int tipPoarta; // 1-AND; 2-OR; 3-NOT; -1 daca nu e poarta
    struct nod *parinte;
    struct nod **fii;
} TNod, *TArb;

//  Functii pentru arbore
TArb AlocaNod();
void DistrArb(TArb *a);
void ContruireArbore(TArb a, int **portiInfo, int nrPorti, int nrInputs, int index);
void EliminarePortiNOT(TArb a, TArb parinte, int underNegation);
void EliminaPortileInutile(TArb a, TArb parinte, int tipPoarta);
void ReduceLaCNFSAT(TArb a, TArb parinte, int outputIdx);
void AfisareClauze(TArb a, FILE *write);
void InitializareParinti(TArb a, TArb parinte);