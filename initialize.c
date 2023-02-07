#include "scan.h"


int Putback = '\n';
int Line = 1;
FILE *Infile;
FILE *Outfile;
struct token Token;
char *Text;
struct symtable *Gsym[1024];