/*! \file computePatchOpt.c
 *  \brief	   This implements the computePatchOpt program.
 *  \author    Duhart Claudia 
 *  \author    Martinez Cléa
 *  \version   1.0
 *  \date      2015
 *  \warning   Usage: computePatchOpt input_file output_file
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

enum op {
    COPIE = 'c', //non affiché ici, on aurait pu mettre '\0'
    SUBS = '=',
    DEL = 'd',
    DELM = 'D',
    ADD = '+'
};

//#define min2(a,b) ((a<=b)?a:b)

int min(int a, int b, enum op opa, enum op opb, enum op *opmin) {
    if (a < b) {
        *opmin = opa;
        return a;
    } else {
        *opmin = opb;
        return b;
    }
}

struct line {
    uint32_t numLine;
    uint32_t nbChar;
    struct line *nxt;

};

struct operation {
    enum op operation;
    uint32_t i;
    uint32_t j;
};

struct line *initList() {
    struct line *lines = malloc(sizeof (struct line));
    if (lines) /* si l'allocation a réussi */ {
        lines->numLine = 0; /* la ligne 0 correspond à une ligne virtuelle constituant le début du fichier*/
        lines->nbChar = 0;
        lines->nxt = NULL; /* affectation du champ nxt à la liste vide */
    }
    return lines; /* retour de la liste (correctement allouée et affectée ou NULL) */
}

void addNext(struct line *lines, uint32_t nbChar, uint32_t num) {
    lines->nxt = malloc(sizeof (struct line));
    if (lines->nxt) {
        lines->nxt->numLine = num;
        lines->nxt->nbChar = nbChar;
        lines->nxt->nxt = NULL;
    }
    // sinon problème de malloc
}

uint32_t listLines(struct line *lines, FILE *file) {
    int c;
    uint32_t nbLines = 0;
    uint32_t nbCharPerLine = 0;
    uint32_t c2 = '\0';
    struct line *current = lines;

    while ((c = fgetc(file)) != EOF) {

        if (c == '\n') {
            nbCharPerLine++;
            nbLines++;
            addNext(current, nbCharPerLine, nbLines);
            nbCharPerLine = 0;
            current = current->nxt;
        } else {
            c2 = c;
            nbCharPerLine++;
        }
    }

    /* Ici, c2 est égal au caractère juste avant le EOF. */
    if (c2 != '\n') {
        nbLines++; /* Dernière ligne non finie */
        addNext(current, nbCharPerLine + 1, nbLines);
    }

    return nbLines;
}

/* renvoie le nb de caractères de la ligne i*/
// TO DO : lever une exception si i> nb lignes du fichier ? 

uint32_t getNbCar(uint32_t i, struct line *lines, uint32_t *somme) {
    *somme = 0;
    struct line *current = lines;
    uint32_t k = 0;
    while (k != i) {
        *somme += current->nbChar;
        current = current->nxt;
        k++;
    }

    return current->nbChar;
}

bool compareLigne(FILE *inputFile, FILE *outputFile, uint32_t nbCar) {
    uint32_t i = 1;
    char c1;
    char c2;

    while (i < nbCar) {
        c1 = fgetc(inputFile);
        c2 = fgetc(outputFile);
        if (c1 != c2)
            return false;
        i++;
    }

    return true;
}

bool egalite(uint32_t i, uint32_t j, struct line *lines1, struct line *lines2, FILE *inputFile, FILE *outputFile) {
    /* s1 & s2 contiennent respectivement le nombre de caractères avant la ligne i (j) dans le fichier 1 (2)*/
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    uint32_t nbCarI = getNbCar(i, lines1, &s1);
    uint32_t nbCarJ = getNbCar(j, lines2, &s2);
    if (nbCarI == nbCarJ) {
        // TO DO : comparer le contenu des deux lignes
        fseek(inputFile, s1, SEEK_SET);
        fseek(outputFile, s2, SEEK_SET);
        return compareLigne(inputFile, outputFile, nbCarI);
    }

    return false;
}

/*renvoie f(i,j) selon l'équation de Bellman*/
uint32_t minimum(uint32_t i, uint32_t j, uint32_t **tab, enum op **top, struct line *lines1, struct line *lines2, FILE *inputFile, FILE *outputFile, uint32_t minCol) {
    uint32_t *s = malloc(sizeof (uint32_t));
    enum op opmin;

    uint32_t m = 10 + tab[i - 1][j];
    m = min(minCol, m, DELM, DEL, &opmin);
    m = min(m, 10 + getNbCar(j, lines2, s) + tab[i][j - 1], opmin, ADD, &opmin);
    m = min(m, 10 + getNbCar(j, lines2, s) + tab[i - 1][j - 1], opmin, SUBS, &opmin);

    if (egalite(i, j, lines1, lines2, inputFile, outputFile)) {
        //printf("ligne %i de input et %i de output égales \n", i,j);
        m = min(m, tab[i - 1][j - 1], opmin, COPIE, &opmin);
    }
    top[i][j] = opmin;
    return m;
}

int buildPath(uint32_t L1, uint32_t L2, enum op **Top, struct operation path[]) {

    int i = L1;
    int j = L2;

    uint32_t ind = 0;
    while ((i != 0) || (j != 0)) {

        //DELETION
        if (Top[i][j] == DEL) {
            path[ind].operation = DEL;
            i--;
            //printf("DEL HAUT!! \n");
        }
            //ADDITION
        else if (Top[i][j] == ADD) {
            path[ind].operation = ADD;
            j--;
            //printf("ADD GAUCHE\n");
        }
            //COPIE
        else if (Top[i][j] == COPIE) {
            path[ind].operation = COPIE;
            i--;
            j--;
            //printf("copie DIAG \n ");
        }            //SUBS
        else {
            path[ind].operation = SUBS;
            i--;
            j--;
            //printf("subs DIAG \n");
        }


        path[ind].i = i;
        path[ind].j = j;
        ind++;
    }

    // si il ne reste que des additions à faire
    if (i == 0 && j != 0) {
        while (j != 0) {
            path[ind].operation = ADD;
            path[ind].i = i;
            path[ind].j = j;
            ind++;
            j--;
            //printf("add finale GAUCHE \n");
        }
    }        // si il ne reste que des deletions
    else if (j == 0 && i != 0) {
        while (i != 0) {
            path[ind].operation = DEL;
            path[ind].i = i;
            path[ind].j = j;
            ind++;
            i--;
            //printf("del finale HAUT \n");
        }
    }

    // TO DO : parcouri le tableau et écire dans le fichier les op corespondantes
    //         ne pas oublier que quand on a un del, il faut vérifie si on a un autre del apres pour faire une del multiple
    //printf("END BUILD PATH \n");

    //printf("PATH \n");

    return ind;
    //AFFICAGE DU PATH
    /* for(uint32_t k =0; k<ind; k++ ){ */
    /*   printf("k=%i, i=%i , j=%i \n", k,path[k].i, path[k].j ); */
    /* }; */


}

void printLine(int numLine, struct line *lines, FILE* outputFile) {
    int nbCar;
    char c;
    uint32_t somme;
    nbCar = getNbCar(numLine, lines, &somme); // on reparcourt toute la liste à chaque fois, à changer
    fseek(outputFile, somme, SEEK_SET); // on part du début du fichier à chaque fois (pas top) 
    for (int i = 0; i < nbCar; i++) {
        c = fgetc(outputFile);
        if (c!=EOF) printf("%c", c); // on pas affiche EOF
    }
}

void printPatch(struct operation path[], struct line *lines2, FILE* outputFile, int ind) {
    // parcours du tableau et écriture dans le patch
    rewind(outputFile); //on remet le pointeur au debut du fichier
    int k = ind;
    while (k >= 0) {

        // si substitution
        if (path[k].operation == SUBS) {
            //ecrire "+path[k].i\n"
            printf("%c %u\n", (char) SUBS, path[k].i+1);
            // recopier ligne path[k].j du fichier 2 
            printLine(path[k].j + 1, lines2, outputFile);
        }            // si addition
        else if (path[k].operation == ADD) {

	   //ecrire "+path[k].i\n"
            printf("%c %u\n", (char) ADD, path[k].i);	   
            // recopier ligne path[k].j du fichier 2
            printLine(path[k].j + 1, lines2, outputFile);
        } else if (path[k].operation == DEL) {
            // si deletion simple
            if (k == 0 || path[k - 1].operation != DEL) {
                // ecrire "dpath[k]\n"
                printf("%c %u\n", (char) DEL, path[k].i+1);
            } else {
                int compteurdel = 1;
                while ((k - compteurdel) >= 0 && (path[k - compteurdel].operation == DEL)){
                    compteurdel++;
		}
                    //ecrire D path[k].i compteurdel \n 
                    printf("%c %u %u\n", (char) DELM, path[k].i+1, compteurdel);
		    k += (1 - compteurdel);
                
            }
        }
        // else=copie, on fait rien on passe a la case suivante
        k--;
        //printf("k=%i", k); 
    }
}

void minColonne(uint32_t a, uint32_t b, uint32_t posA, uint32_t posB, uint32_t *min, uint32_t *posmin) {
    if (a < b) {
        *min = a;
        *posmin = posA;
    } else {
        *min = b;
        *posmin = posB;
    }
}

// TODO : gérer fichier videsur l'init de tab

uint32_t computePatchOpt(FILE *inputFile, FILE *outputFile) {
    uint32_t *s = malloc(sizeof (uint32_t));

    /* on stocke le nombre de lignes de chaque fichier ainsi que
       le nombre de caractères de chaque ligne */
    struct line *lines1 = initList();
    struct line *lines2 = initList();
    uint32_t nbLines1 = listLines(lines1, inputFile);
    uint32_t nbLines2 = listLines(lines2, outputFile);
    //printf("L1=%i , L2=%i \n", nbLines1, nbLines2);

    /*Allocation de tableaux */
    /*pour le coût :*/
    uint32_t **Tab = malloc((nbLines1 + 1) * sizeof (uint32_t*));
    /* on stocke aussi l'opération de coût minimum à chaque fois*/
    enum op **Top = malloc((nbLines1 + 1) * sizeof (enum op*));
    assert(Tab != NULL);
    assert(Top != NULL);
    for (uint32_t i = 0; i < nbLines1 + 1; i++) {
        Tab[i] = malloc((nbLines2 + 1) * sizeof (uint32_t));
        assert(Tab[i] != NULL);
        Top[i] = malloc((nbLines2 + 1) * sizeof (enum op));
        assert(Top[i] != NULL);
    }
    
    /*Conditions initiales*/
    Tab[0][0] = 0;
    Top[0][0] = COPIE; //non significatif
    Tab[1][0] = 10; //ATTENTION SI FICHIER VIDE INIT INCORRECTE  
    Top[1][0] = DEL;


    for (uint32_t i = 2; i < nbLines1 + 1; i++) {
        // fprintf(stderr,"la ligne %i de input contient %i car \n", i, getNbCar(i, lines1,s));
        Tab[i][0] = 15;
        Top[i][0] = DELM;
    }


    for (uint32_t j = 1; j < nbLines2 + 1; j++) {
        Tab[0][j] = 10 + getNbCar(j, lines2, s) + Tab[0][j - 1];
        Top[0][j] = ADD;
        //printf("la ligne %i de output contient %i car \n", j, getNbCar(j, lines2,s));
    }

    uint32_t minCol;
    uint32_t pos = 0;
    for (uint32_t j = 1; j < nbLines2 + 1; j++) {
        minColonne(15, Tab[0][j], 0, j, &minCol, &pos);
        for (uint32_t i = 1; i < nbLines1 + 1; i++) {
            minColonne(Tab[i][j], minCol, j, pos, &minCol, &pos);
            Tab[i][j] = minimum(i, j, Tab, Top, lines1, lines2, inputFile, outputFile, minCol);
            //printf("%c | ", (char) Top[i][j]);
        }
        //printf("\n");
    }


    //AFFICHAGE TABLEAU
    //printf("TAB : \n");
    //for (uint32_t i = 0; i < nbLines1 + 1; i++) {
        //for (uint32_t j = 0; j < nbLines2 + 1; j++) {
            //printf(" %i     ", Tab[i][j]);
        //}
        //printf("\n");
    //};


    //printf("cout minimal Tab[%i][%i]= %i \n", nbLines1, nbLines2, Tab[nbLines1][nbLines2]);

    // TO DO : stocker le chemin en remontant dans le tableau
    // générer le patch 
    struct operation path[nbLines1 + nbLines2 + 2];
    //regrouper Top et path??
    int ind = buildPath(nbLines1, nbLines2, Top, path);
    printPatch(path, lines2, outputFile, ind);
    return 0;


}

int main(int argc, char *argv[]) {
    FILE *inputFile;
    FILE *outputFile;
    //FILE *patch ; 

    if (argc < 4) {
        fprintf(stderr, "!!!!! Usage: ./computePatchOpt inputFile outputFile patch !!!!!\n");
        exit(EXIT_FAILURE); /* indicate failure.*/
    }

    inputFile = fopen(argv[1], "r");
    outputFile = fopen(argv[2], "r");
    //patch=fopen(argv[3], "w+");

    if (inputFile == NULL) {
        fprintf(stderr, "!!!!! Error opening inputFile !!!!! \n");
        exit(EXIT_FAILURE);
    }
    if (outputFile == NULL) {
        fprintf(stderr, "!!!!! Error opening outputFile !!!!!\n");
        exit(EXIT_FAILURE);
    }
    //if (patch==NULL) {fprintf (stderr, "!!!!! Error opening patch !!!!!\n"); exit(EXIT_FAILURE);}
    //printf("OUVERTURE FICHIERS \n");
    //printf("COMPUTE PATCH \n ");
    computePatchOpt(inputFile, outputFile);
    //printf("END COMPUTE PATCH \n ");
    fclose(inputFile);
    fclose(outputFile);
    //fclose(patch);
    //FREEEEEEEEEEEEEEE TAS RIEN COMPRIS
    return 0;
}
