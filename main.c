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
    struct ASTnode *tree;

    if (argc != 2)
        usage(argv[0]);

    Text = (char*) malloc(513); 
    for (int i = 0; i < 1024; i ++) {
        Gsym[i] = (struct symtable *)malloc(sizeof(struct symtable));
    }

    init();

    // Open up the input file
    if ((Infile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    // Create the output file
    if ((Outfile = fopen("out.s", "w")) == NULL) {
        fprintf(stderr, "Unable to create out.s: %s\n", strerror(errno));
        exit(1);
    }

    //scanfile();
    scan(&Token);                           // Get the first token from the input
    genpreamble();                          // Output the preamble
    while (1) {                             // Parse a function and generate 
        tree = function_declaration();      // the assembly code for this
        genAST(tree, NOREG, 0);
        if (Token.token == T_EOF)
            break;
    }
    fclose(Outfile);
    exit(0);
}