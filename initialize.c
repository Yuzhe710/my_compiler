#include "scan.h"

int Putback = '\n';
int Line = 1;
FILE *Infile;
FILE *Outfile;
char *Outfilename;   
struct token Token;
char *Text;
struct symtable *Symtable[1024];

struct symtable *Functionid = NULL;
struct symtable *Globhead = NULL;
struct symtable *Globtail = NULL;
struct symtable *Loclhead = NULL;
struct symtable *Locltail = NULL;
struct symtable *Parmhead = NULL;
struct symtable *Parmtail = NULL;
struct symtable *Comphead = NULL;
struct symtable *Comptail = NULL;
int O_dumpAST;
int O_keepasm;           
int O_assemble;          
int O_dolink;            
int O_verbose;         
int Globs;
int Locls;