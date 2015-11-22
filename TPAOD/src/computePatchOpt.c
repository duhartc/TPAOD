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

#define min(a,b) ((a<=b)?a:b)

struct line
{
    int numLine;
    int nbChar;
    struct line *nxt;

};

struct line *initList()
{
   struct line *lines = malloc(sizeof(struct line));
   if (lines)                           /* si l'allocation a réussi */
   {
       lines->numLine = 0;              /* la ligne 0 correspond à une ligne virtuelle constituant le début du fichier*/
       lines->nbChar = 0;
       lines->nxt = NULL;              /* affectation du champ nxt à la liste vide */
   }
   return lines;                        /* retour de la liste (correctement allouée et affectée ou NULL) */
}

void addNext(struct line *lines, long int nbChar, long int num) {
    lines->nxt = malloc(sizeof(struct line));
    if (lines->nxt) {
        lines->nxt->numLine = num;
        lines->nxt->nbChar = nbChar;
        lines->nxt->nxt = NULL;
    }
    // sinon problème de malloc
}

int listLines(struct line *lines, FILE *file)
{
	int c;
	long int nbLines = 0;
        long int nbCharPerLine = 0;
	int c2 = '\0';
        struct line *current = lines; 
 
	while((c=fgetc(file)) != EOF)
	{
		if(c=='\n') {
                    nbCharPerLine++;
                    nbLines++;
                    addNext(current,nbCharPerLine,nbLines);
                    nbCharPerLine = 0;
                    current = current->nxt;
                }
                nbCharPerLine++;
		c2 = c;
	}
 
	/* Ici, c2 est égal au caractère juste avant le EOF. */
	if(c2 != '\n')
		nbLines++; /* Dernière ligne non finie */
 
	return nbLines;
}
 
/* renvoie le nb de caractères de la ligne i*/ 
// TO DO : lever une exception si i> nb lignes du fichier ? 
int getNbCar(int i, struct line *lines, int *somme){
  *somme = 0;
  struct line *current=lines ; 
  int k=1;
  while(k!=i){ 
    *somme += current->nbChar;
    current=current->nxt ;
    k++ ;     
  }
  return current->nbChar ; 
}

bool compareLigne(FILE *inputFile, FILE *outputFile, int nbCar){
    int i=1 ;
    char c1 ; 
    char c2; 
    
    while (i<nbCar){
        c1=fgetc(inputFile);
        c2=fgetc(outputFile);
        if (c1!=c2)
            return false ;
        i++;
    }   
    
    return true;
}


bool egalite(int i, int j, struct line *lines1, struct line *lines2, FILE *inputFile, FILE *outputFile){
  /* s1 & s2 contiennent respectivement le nombre de caractères avant la ligne i (j) dans le fichier 1 (2)*/
  int s1 = 0;
  int s2 = 0;
  int nbCarI = getNbCar(i,lines1, &s1);
  int nbCarJ = getNbCar(j, lines2, &s2);
  if (nbCarI==nbCarJ){
    // TO DO : comparer le contenu des deux lignes
    fseek(inputFile, s1, SEEK_SET );
    fseek(outputFile, s2, SEEK_SET );
    return compareLigne(inputFile, outputFile, nbCarI);
  }

  return false ; 
}

/*renvoie f(i,j) selon l'équation de Bellman*/
long int minimum(int i, int j, size_t n, long int Tab[][n],struct line *lines1, struct line *lines2, FILE *inputFile, FILE *outputFile){
  int *s= 0; ;
  long int m = 10+Tab[i-1][j];
  for (int k=2 ; k<=i ; k++){
    m=min(15+Tab[i-k][j], m);
  } ;
  m=min(m, 10+getNbCar(j, lines2,s)+Tab[i][j-1]) ;
  m=min(m, 10+getNbCar(j, lines2,s)+Tab[i-1][j-1]);

  if (egalite(i,j, lines1, lines2, inputFile, outputFile))
    m=min(m, Tab[i-1][j-1]);

  return m; 
}


int computePatchOpt(FILE *inputFile, FILE *outputFile){
    struct line *lines1 = initList();
    struct line *lines2 = initList(); 
    int *s = 0 ;
    
    /* on stocke le nombre de lignes de chaque fichier ainsi que
     le nombre de caractères de chaque ligne */
    long int nbLines1 = listLines(lines1, inputFile);
    long int nbLines2 = listLines(lines2, outputFile);
    
    long int Tab[nbLines1][nbLines2];
    /* initialisation du tableau */
    Tab[0][0]=0;
    Tab[1][0]=10;

    for (long int i=2; i<nbLines1; i++) {
        Tab[i][0] = 15;
    }
    for (long int j=1; j<nbLines2; j++) {
      Tab[0][j] = 10 + getNbCar(j,lines2,s);
    }
    
    for (long int i=1; i<nbLines1; i++) {
        for (long int j=1; j<nbLines2; j++) {
	      Tab[i][j]= minimum(i,j,nbLines2,Tab, lines1, lines2, inputFile, outputFile);
            }
    }
    return 0;
}


int main(int argc, char *argv[]){
        FILE *inputFile;
	FILE *outputFile;
	
	if(argc<3){
		fprintf(stderr, "!!!!! Usage: ./computePatchOpt inputFile outputFile !!!!!\n");
	    exit(EXIT_FAILURE); /* indicate failure.*/
	}

	inputFile = fopen(argv[1] , "r" );
	outputFile = fopen(argv[2] , "r" );
	
	if (inputFile==NULL) {fprintf(stderr, "!!!!! Error opening inputFile !!!!! \n"); exit(EXIT_FAILURE);}
	if (outputFile==NULL) {fprintf (stderr, "!!!!! Error opening outputFile !!!!!\n"); exit(EXIT_FAILURE);}

	computePatchOpt(inputFile, outputFile);

	fclose(inputFile);
	fclose(outputFile);
	return 0;
}
