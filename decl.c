#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse the declaration of variable
// It must be the 'int' token followed by an identifier 
// and a semicolon. Add the identifier to symbol table
void var_declaration(void) {

    match(T_INT, "int");
    matchident();
    addglob(Text);
    genglobsym(Text);
    matchsemi();
}