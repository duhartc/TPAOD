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


struct line {
    uint32_t numLine;
    uint64_t posLine;
    uint32_t nbChar;
    struct line *nxt;

};

struct operation {
    enum op operation;
    uint32_t i;
    uint32_t j;
    uint32_t originalI;
};


/*!
 *Fonction minimum
 * @param a premier entier à comparer
 * @param b deuxième entier à comparer
 * @param opa operation correspondant à a
 * @param opb operation correspondant à b
 * @param opmin operation correspondant au min de (a,b)
 * \returns min(a,b)
 */
int min(int a, int b, enum op opa, enum op opb, enum op *opmin) {
    if (a < b) {
        *opmin = opa;
        return a;
    } else {
        *opmin = opb;
        return b;
    }
}

/*!
 * fonction qui initialise une liste chaînée 
 * \returns {une liste chaînée avec sentinelle}
 */
struct line *initList() {
    struct line *lines = malloc(sizeof (struct line));
    if (lines){ // si l'allocation a réussi/ 
        lines->numLine = 0; // la ligne 0 correspond à une ligne virtuelle constituant le début du fichier
        lines->nbChar = 0;
        lines->posLine = 0;
        lines->nxt = NULL; // affectation du champ nxt à la liste vide 
    }
    return lines; // retour de la liste (correctement allouée et affectée ou NULL) 
}


/*!
 * fonction qui ajoute un élément à une liste chaînée
 * @param lines  liste chaînée à laquelle ajouter un élément struct line
 * @param nbChar nombre de caractères de la ligne à ajouter
 * @param num numéro de la ligne à ajouter
 */

void addNext(struct line *lines, uint32_t nbChar, uint32_t num, uint64_t posLine) {
    lines->nxt = malloc(sizeof (struct line));
    if (lines->nxt) {
        lines->nxt->numLine = num;
        lines->nxt->nbChar = nbChar;
        lines->nxt->posLine = posLine;
        lines->nxt->nxt = NULL;
    }
    // sinon problème de malloc
}


/*!
 * fonction qui libère une struct line **
 * @param tabLines élément à libérer
 */
void  freeList(struct line **tabLines)
{
    struct line *tmp = tabLines[0];
    struct line *tmpnxt;
 
    /* Tant que l'on n'est pas au bout de la liste */
    while(tmp != NULL)
    {    tmpnxt = tmp->nxt;      
      free(tmp);
        tmp = tmpnxt;
    }
    
    free(tabLines);
}


/*!
 * fonction qui libère les tableaux tab et top
 * @param tab : tableau à libérer
 * @param top : tableau à libérer
 * @param L1 : dimension 1 des tableaux
 */
void freeTab(uint32_t **Tab,enum op **Top, uint32_t L1){
  for (uint32_t i=0; i<L1+1; i++){
    free(Tab[i]); 
    free(Top[i]);
  }
  free(Tab); 
  free(Top); 
}

/*!
 * fonction qui remplit une liste avec les données d'un fichier 
 * @param lines liste de lignes dans laquelle on stocke les données
 * @param file fichier duquel on extrait les données
 * \returns nombre de lignes du fichier
 */
struct line **listLines(struct line *lines, uint32_t *count, FILE *file) {
    int c;
    uint32_t nbLines = 0;
    uint32_t nbCharPerLine = 0;
    uint64_t actualPos = ftell(file);
    uint32_t c2 = '\0';
    struct line *current = lines;

    while ((c = fgetc(file)) != EOF) {

        if (c == '\n') {
            nbCharPerLine++;
            nbLines++;
            addNext(current, nbCharPerLine, nbLines, actualPos);
            nbCharPerLine = 0;
            current = current->nxt;
            actualPos = ftell(file);
        } else {
            c2 = c;
            nbCharPerLine++;
        }
    }

    /* Ici, c2 est égal au caractère juste avant le EOF. */
    if (c2 != '\n') {
        nbLines++; /* Dernière ligne non finie */
        addNext(current, nbCharPerLine + 1, nbLines, actualPos);
    }
    
    struct line **tab = malloc(sizeof(struct line*)*(nbLines + 1));
    current = lines;
    for (uint32_t i = 0; i < nbLines + 1; i++) {
        tab[i] = current;
        current = current->nxt;
    }

    *count = nbLines;
    return tab;
}


/*!
 * fonction qui récupère le nombre de caractères d'une ligne 
 * @param i numéro de la ligne dont on veut le nombre de caractères
 * @param liste de la ligne dans laquelle se trouve la ligne
 * @param somme stocke le nombre de caractères du fichier avant le début de la ligne
 * \returns nombre de caractères de la ligne 
 */
uint32_t getNbCar(uint32_t i, struct line **lines, uint32_t *somme) {
        
    *somme = lines[i]->posLine;

    return lines[i]->nbChar;
}


/*!
 * fonction qui teste l'égalité entre deux lignes 
 * requiert : les lignes testées ont le même nombre de caractères
 * @param inputFile fichier F1
 * @param outputFile fichier F2
 * @param nbCar nombre de caractères des lignes
 * \returns {true si les lignes sont égales, faux sinon} 
 */
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


/*!
 * fonction qui teste l'égalité entre deux lignes d
 * @param i première ligne testée
 * @param j deuxième ligne testée
 * @param lines1 liste des lignes contenant la ligne 1
 * @param lines2 liste des lignes contenant la ligne 2 
 * @param inputFile fichier F1
 * @param outputFile fichier F2
 * \returns {true si les lignes sont égales, faux sinon} 
 */
bool egalite(uint32_t i, uint32_t j, struct line **lines1, struct line **lines2, FILE *inputFile, FILE *outputFile) {
    /* s1 & s2 contiennent respectivement le nombre de caractères avant la ligne i (j) dans le fichier 1 (2)*/
    uint32_t s1 = 0;
    uint32_t s2 = 0;
    uint32_t nbCarI = getNbCar(i, lines1, &s1);
    uint32_t nbCarJ = getNbCar(j, lines2, &s2);
    if (nbCarI == nbCarJ) {
         fseek(inputFile, s1, SEEK_SET);
        fseek(outputFile, s2, SEEK_SET);
        return compareLigne(inputFile, outputFile, nbCarI);
    }

    return false;
}

/*!
 * fonction qui calcule le coût V(i,j) selon Bellman
 * @param i 1e paramètre de V(i,j)
 * @param j 2e paramètre de V(i,j)
 * @param tab tableau des V(i,j)
 * @param top tableau des opérations
 * @param lines1 liste contenant la ligne i
 * @param lines2 liste contenant la ligne j
 * @param inputFile fichier contenant la ligne i
 * @param outputFile fichier contenant la ligne j
 * @param minCol minimum d'une colonne du tableau
 * @param posMinCol position du minimum de la colonne
 * \returns {V(i,j)} 
 */
uint32_t minimum(uint32_t i, uint32_t j, uint32_t **tab, enum op **top, struct line **lines1, struct line **lines2, FILE *inputFile, FILE *outputFile, uint32_t *minCol, uint32_t *posMinCol) {
    uint32_t *s = malloc(sizeof (uint32_t));
    enum op opmin;

    uint32_t m = 10 + tab[i - 1][j];
    m = min(15 + *minCol, m, DELM, DEL, &opmin);
    m = min(m, 10 + getNbCar(j, lines2, s) + tab[i][j - 1], opmin, ADD, &opmin);
    m = min(m, 10 + getNbCar(j, lines2, s) + tab[i - 1][j - 1], opmin, SUBS, &opmin);

    if (egalite(i, j, lines1, lines2, inputFile, outputFile)) {       
        m = min(m, tab[i - 1][j - 1], opmin, COPIE, &opmin);
    } 
    top[i][j] = opmin;
    
    if (m < *minCol) {
        *minCol = m;
        *posMinCol = i;
    }
    free(s);
    return m;
}


/*!
 * fonction qui stocke les opérations du patch de coût minimal
 * @param L1 nombre de lignes du fichier 1
 * @param L2 nombre de lignes du fichier 2 
 * @param top tableau des opérations
 * @param path tableau contenant les opérations du patch minimal
 * \returns {l'indice de la dernière case modifiée du tableau path} 
 */
int buildPath(uint32_t L1, uint32_t L2, enum op **Top, struct operation path[], uint32_t tMin[]) {

    int i = L1;
    int j = L2;

    uint32_t ind = 0;
    while ((i != 0) || (j != 0)) {

        //DELETION
        if (Top[i][j] == DEL) {
            path[ind].operation = DEL;
            i--;
            
        }
            //ADDITION
        else if (Top[i][j] == ADD) {
            path[ind].operation = ADD;
            j--;
            
        }
            //COPIE
        else if (Top[i][j] == COPIE) {
            path[ind].operation = COPIE;
            i--;
            j--;
            
        }
        else if (Top[i][j] == DELM) {
            path[ind].operation = DELM;
            path[ind].originalI = i;
            i = tMin[j];
        }
        //SUBS
        else {
            path[ind].operation = SUBS;
            i--;
            j--;
            
        }


        path[ind].i = i;
        path[ind].j = j;
        ind++;
    }

    // s'il ne reste que des additions à faire
    if (i == 0 && j != 0) {
        while (j != 0) {
            path[ind].operation = ADD;
            path[ind].i = i;
            path[ind].j = j;
            ind++;
            j--;
          
        }
    }        // si il ne reste que des deletions
    else if (j == 0 && i != 0) {
        while (i != 0) {
            path[ind].operation = DEL;
            path[ind].i = i;
            path[ind].j = j;
            ind++;
            i--;
   
        }
    }


    return ind - 1;
  

}

/*!
 * fonction qui écrit une ligne dans la sortie standard
 * @param numLine numéro de la ligne à copier
 * @param lines liste contenant les données de la ligne à copier 
 * @param outputFile fichier contenant la ligne à copier 
 */
void printLine(int numLine, struct line **lines, FILE* outputFile) {
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


/*!
 * fonction qui écrit le patch dans la sortie standard
 * @param path tableau contenant les opérations du patch
 * @param lines 2 liste contenant les lignes du fichier de sortie
 * @param outputFile fichier de sortie
 * @param ind indice de la derniere case significative de path
 */
void printPatch(struct operation path[], struct line **lines2, FILE* outputFile, int ind, uint32_t tMin[]) {
    // parcours du tableau et écriture dans le patch
    rewind(outputFile); //on remet le pointeur au debut du fichier
    int k = ind;
    while (k >= 0) {

        // si substitution
        if (path[k].operation == SUBS) {
           
            printf("%c %u\n", (char) SUBS, path[k].i+1);
         
            printLine(path[k].j + 1, lines2, outputFile);
        }            // si addition
        else if (path[k].operation == ADD) {	   
            printf("%c %u\n", (char) ADD, path[k].i); 
            printLine(path[k].j + 1, lines2, outputFile);
        } else if (path[k].operation == DEL) {
            // si deletion simple
            
                      printf("%c %u\n", (char) DEL, path[k].i+1);
            
        }
        else if (path[k].operation == DELM) {
                int compteurdel = path[k].originalI - tMin[path[k].j];
               
                    printf("%c %u %u\n", (char) DELM, path[k].i+1, compteurdel);
		   
        }
        // else=copie, on fait rien on passe a la case suivante
        k--;
        
        
    }
}



/*!
 * fonction qui calcule un mimum et met à jour les param pos 
 * @param a entier à comparer
 * @param b entier à comparer
 * @param posA position de a dans la colonne
 * @param posB position de b dans la colonne
 * @param min : minimum de a et b
 * @param posmin position du minmum de a et b
 * \returns {0}
 */
void minColonne(uint32_t a, uint32_t b, uint32_t posA, uint32_t posB, uint32_t *min, uint32_t *posmin) {
    if (a < b) {
        *min = a;
        *posmin = posA;
    } else {
        *min = b;
        *posmin = posB;
    }
}


/*!
 * fonction qui construit le patch 
 * @param path tableau contenant les opérations du patch
 * @param intputFile fichier d'entrée
 * @param outputFile fichier de sortie
 * \returns {0}
 */
uint32_t computePatchOpt(FILE *inputFile, FILE *outputFile) {
    uint32_t *s = malloc(sizeof (uint32_t));

    /* on stocke le nombre de lignes de chaque fichier ainsi que
       le nombre de caractères de chaque ligne */
    struct line *lines1 = initList();
    struct line *lines2 = initList();
    uint32_t nbLines1, nbLines2;
    struct line **tabLines1 = listLines(lines1, &nbLines1, inputFile);
    struct line **tabLines2 = listLines(lines2, &nbLines2, outputFile);
 

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
    Tab[1][0] = 10;   
    Top[1][0] = DEL;


    for (uint32_t i = 2; i < nbLines1 + 1; i++) {
        Tab[i][0] = 15;
        Top[i][0] = DELM;
    }


    for (uint32_t j = 1; j < nbLines2 + 1; j++) {
        Tab[0][j] = 10 + getNbCar(j, tabLines2, s) + Tab[0][j - 1];
        Top[0][j] = ADD;
    }
    
    uint32_t *tPosMinCol = malloc((nbLines2 + 1) * sizeof (uint32_t));
    uint32_t minCol;
    for (uint32_t j = 1; j < nbLines2 + 1; j++) {
        minCol= Tab[0][j];
        tPosMinCol[j] = 0;
         for (uint32_t i = 1; i < nbLines1 + 1; i++) {
            
            Tab[i][j] = minimum(i, j, Tab, Top, tabLines1, tabLines2, inputFile, outputFile, &minCol, &tPosMinCol[j]);
            
        }
       
    }

    fprintf(stderr, "cout minimal Tab[%i][%i]= %i \n", nbLines1, nbLines2, Tab[nbLines1][nbLines2]);

 
    // générer le patch 
    struct operation path[nbLines1 + nbLines2 + 2];
     int ind = buildPath(nbLines1, nbLines2, Top, path, tPosMinCol);
    printPatch(path, tabLines2, outputFile, ind, tPosMinCol);

    // Libération de la mémoire
    free(s); 
    freeList(tabLines1); 
    freeList(tabLines2); 
    freeTab(Tab,Top,  nbLines1);
    free(tPosMinCol);
    return 0;


}



/**
 * Fonction main
 * requiert détre appelée avec deux arguments : fichierSource et fichierCible 
 * \param argc nombre d'arguments en ligne de commande
 * \param argv vecteur argument des arguments de ligne de commande
 * \returns {0 si succès, quite le code sinon} 
*/
int main(int argc, char *argv[]) {
    FILE *inputFile;
    FILE *outputFile; 

    if (argc < 3) {
        fprintf(stderr, "!!!!! Usage: ./computePatchOpt inputFile outputFile !!!!!\n");
        exit(EXIT_FAILURE); /* indicate failure.*/
    }

    inputFile = fopen(argv[1], "r");
    outputFile = fopen(argv[2], "r");


    if (inputFile == NULL) {
        fprintf(stderr, "!!!!! Error opening inputFile !!!!! \n");
        exit(EXIT_FAILURE);
    }
    if (outputFile == NULL) {
        fprintf(stderr, "!!!!! Error opening outputFile !!!!!\n");
        exit(EXIT_FAILURE);
    }
    computePatchOpt(inputFile, outputFile);
    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
