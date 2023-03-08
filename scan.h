// #ifndef extern_
//  #define extern_ extern
// #endif

#include <stdio.h>
// extern_ int Putback;
// extern_ int Line;
// extern_ FILE *Infile;
// extern_ struct token Token;
// #define TEXTLEN     512 

extern int Putback;             // Current line number
extern int Line;                // Character put back by scanner
extern FILE *Infile;            // Input and output files
extern FILE *Outfile;           
extern struct token Token;      // Last token scanned
extern char *Text;              // a buffer contains Last identifier scanned
extern struct symtable *Symtable[1024];   // Gobal symbol table
extern int Globs;               // Position of next free global symbol slot
extern int Locls;               // Position of next free local symbol slot
extern int Functionid;          // Symbol id of the current function
extern int O_dumpAST;
// #ifndef extern_
// #define extern_ extern
// #endif

// // Global variables
// // Copyright (c) 2019 Warren Toomey, GPL3

// extern_ int Line;			// Current line number
// extern_ int Putback;			// Character put back by scanner
// extern_ FILE *Infile;			// Input and output files
// extern_ FILE *Outfile;
// extern_ struct token Token;		// Last token scanned
// extern_ char Text[TEXTLEN + 1];		// Last identifier scanned