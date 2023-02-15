#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse the current token and 
// return a primitive type enum value
int parse_type(int t) {
    if (t == T_CHAR) return P_CHAR;
    if (t == T_INT) return P_INT;
    if (t == T_VOID) return P_VOID;
    fatald("Illegal type, token", t);
}

// Parse the declaration of variable
// It must be the 'int' token followed by an identifier 
// and a semicolon. Add the identifier to symbol table
void var_declaration(void) {
    int id, type;

    type = parse_type(Token.token);
    scan(&Token);
    matchident();
    id = addglob(Text,type, S_VARIABLE);
    genglobsym(id);
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
    nameslot = addglob(Text, P_VOID, S_FUNCTION);
    matchlparen();
    matchrparen();

    // Get the AST for the body
    tree = compound_statement();

    // Return an A_FUNCTION node which has the function's nameslot
    // and the compound statement sub-tree
    return mkastunary(A_FUNCTION, P_VOID, tree, nameslot);
}