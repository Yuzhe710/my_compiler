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
extern char Text[512 + 1];  // a buffer contains Last identifier scanned