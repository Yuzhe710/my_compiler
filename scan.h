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
extern char *Outfilename;       // Name of file we opened as Outfile           
extern struct token Token;      // Last token scanned
extern char *Text;              // a buffer contains Last identifier scanned
extern struct symtable *Symtable[1024];   // Gobal symbol table
extern int Globs;               // Position of next free global symbol slot
extern int Locls;               // Position of next free local symbol slot
extern struct symtable *Functionid;          // Symbol id of the current function
 
// Symbol table lists
extern struct symtable *Globhead, *Globtail;    // Global variables and functions
extern struct symtable *Loclhead, *Locltail;    // Local variables
extern struct symtable *Parmhead, *Parmtail;    // Local parameters
extern struct symtable *Comphead, *Comptail;    // Composite types

extern int O_dumpAST;           // If true, dump the AST tree
extern int O_keepasm;           // If true, keep any assembly files
extern int O_assemble;          // If true, assemble the assembly files
extern int O_dolink;            // If true, link the object files
extern int O_verbose;           // If true, print info on compilation stages
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