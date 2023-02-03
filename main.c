#include "defs.h"
#include "scan.h"
#include "decl.h"
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
char *tokstr[] = {"+", "-", "*", "/", "intlit"};

// loop scannig all the tokens in the input file.
// print out each token found
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

int main(int argc, char *argv[]) {
    if (argc != 2)
        usage(argv[0]);
    
    init();

    if ((Infile = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    scanfile();
    exit(0);

    return 1;
}

