#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse the declaration of variable
// It must be the 'int' token followed by an identifier 
// and a semicolon. Add the identifier to symbol table
void var_declaration(void) {
    // printf("Line is %d\n", Line);
    match(T_INT, "int");
    matchident();
    addglob(Text);
    genglobsym(Text);
    matchsemi();
}

// Parse the declaration of a simplistic function
struct ASTnode *function_declaration(void) {
    struct ASTnode *tree;
    int nameslot;

    // Find 'void', the identifier, and the '(' and ')'
    // For now, do nothing with them
    match(T_VOID, "void");
    matchident();
    nameslot = addglob(Text);
    matchlparen();
    matchrparen();

    // Get the AST for the body
    tree = compound_statement();

    // Return an A_FUNCTION node which has the function's nameslot
    // and the compound statement sub-tree
    return mkastunary(A_FUNCTION, tree, nameslot);
}