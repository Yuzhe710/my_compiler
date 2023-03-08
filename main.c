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
    Globs = 0;
    Locls = NSYMBOLS - 1;
}

// Print out a usage message if arguments are wrong
static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-T] infile\n", prog);
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

// Main program: check arguments and print a usage if we don't have an argument
// Open up the input file and call scanfile() to scan the tokens in it.
int main(int argc, char *argv[]) {
    int i;

    Text = (char*) malloc(513); 
    for (int i = 0; i < 1024; i ++) {
        Symtable[i] = (struct symtable *)malloc(sizeof(struct symtable));
    }

    init();

    // Scan for command options
    for (i=1; i<argc; i++) {
        if (*argv[i] != '-') break;
        for (int j=1; argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'T': O_dumpAST = 1; break;
                default: usage(argv[0]);
            }
        }
    }

    // Ensure we have an input file argument
    if (i >= argc)
        usage(argv[0]);

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

    // For now, ensure that void printint() and printchar() is defined
    addglob("printint", P_CHAR, S_FUNCTION, 0, 0);
    addglob("printchar", P_VOID, S_FUNCTION, 0, 0);

    //scanfile();
    scan(&Token);                           // Get the first token from the input
    genpreamble();                          // Output the preamble
    global_declarations();
    fclose(Outfile);
    return 0;
}