#include "defs.h"
// #define extern_
#include "scan.h"
// #undef extern_
#include "decl.h"
#include "initialize.c"
#include <errno.h>

// Initialise global variables
static void init() {
    Line = 1;
    Putback = '\n';
}

// Print out a usage message if arguments are wrong
static void usage(char *prog) {
    fprintf(stderr, "Usage: %s infile\n", prog);
    exit(1);
}

// List of printable tokens
// char *tokstr[] = {"+", "-", "*", "/", "intlit"};

// loop scannig all the tokens in the input file.
// print out each token found
// this is for Part01 - Lexical Scanner
/*
static void scanfile() {
    struct token T;

    while (scan(&T)) {
        printf("Token %s", tokstr[T.token]);
        if (T.token == T_INT) {
            printf(", value %d", T.intvalue);
        }
        printf("\n");
    }
}
*/

void main(int argc, char *argv[]) {
    struct ASTnode *n;

    if (argc != 2)
        usage(argv[0]);
    
    init();

    if ((Infile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    //scanfile();
    scan(&Token);                       // Get the first token from the input
    n = binexpr();                      // Parse the expression in the file
    printf("%d\n", interpretAST(n));     // calculate the final result
    exit(0);
}

