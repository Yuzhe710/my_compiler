#include "scan.h"

int Putback = '\n';
int Line = 1;
FILE *Infile;
FILE *Outfile;
char *Outfilename;   
struct token Token;
char *Text;
struct symtable *Symtable[1024];

int Functionid = 0;
int O_dumpAST;
int O_keepasm;           
int O_assemble;          
int O_dolink;            
int O_verbose;         
int Globs;
int Locls;