#include "scan.h"

int Putback = '\n';
int Line = 1;
FILE *Infile;
FILE *Outfile;
struct token Token;
char *Text;
struct symtable *Symtable[1024];

int Functionid = 0;
int O_dumpAST;
int Globs;
int Locls;