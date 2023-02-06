#include "scan.h"
#include "decl.h"
#include "defs.h"

// Check that if current token is t
// fetch next token if yes, else throw an error
void match(int t, char *what) {
    if (Token.token == t) {
        scan(&Token);
    } else {
        printf("%s expected on line %d\n", what, Line);
        exit(1);
    }
}

// check if current token is semicolon
void matchsemi(void) {
    match(T_SEMI, ";");
}