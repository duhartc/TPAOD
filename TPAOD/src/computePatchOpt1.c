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

struct line
{
    int numLine;
    int nbChar;
    struct line *nxt;

};

line *initList()
{
   line *lines = malloc(sizeof(line));
   if (line)                           /* si l'allocation a réussi */
   {
       lines->numLine = 0;              /* la ligne 0 correspond à une ligne virtuelle constituant le début du fichier*/
       lines->nbChar = 0;
       lines->nxt = NULL;              /* affectation du champ nxt à la liste vide */
   }
   return lines;                        /* retour de la liste (correctement allouée et affectée ou NULL) */
}

void addNext(line *lines, long int nbChar, long int num) {
    lines->nxt = malloc(sizeof(line));
    if (line->nxt) {
        lines->nxt->numLine = num;
        lines->nxt->nbChar = nbChar;
        lines->nxt->nxt = NULL;
    }
    // sinon problème de malloc
}

int listLines(line *lines, FILE *file)
{
	int c;
	long int nbLines = 0;
        long int nbCharPerLine = 0;
	int c2 = '\0';
        line *current = lines; 
 
	while((c=fgetc(fichier)) != EOF)
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
 

int computePatchOpt(FILE *inputFile, FILE *outputFile){
    line *lines1 = initList();
    line *lines2 = initList(); 
    bool md = false;
    bool d = false;
    
    /* on stocke le nombre de lignes de chaque fichier ainsi que
     le nombre de caractères de chaque ligne */
    long int nbLines1 = listLines(lines1, inputFile);
    long int nbLines2 = listLines(lines2, outputFile);
    
    long int Tab[nbLines1][nbLines2];
    /* initialisation du tableau */
    for (long int i=0; i<nbLines1; i++) {
        Tab[i][0] = 0;
    }
    for (long int j=0; j<nbLines2; j++) {
        Tab[0][j] = 0;
    }
    
    for (long int i=0; i<nbLines1; i++) {
        for (long int j=0; j<nbLines2; j++) {
            if (lines1->nxt->nbChar == lines2->nxt->nbChar){
                //TODO: comparer les lignes entre elles pour voir celles qui sont identiques
            }
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