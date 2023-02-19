#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse the current token and 
// return a primitive type enum value
int parse_type() {
    int type;
    switch (Token.token) {
        case T_VOID: type = P_VOID; break;
        case T_CHAR: type = P_CHAR; break;
        case T_INT:  type = P_INT;  break;
        case T_LONG: type = P_LONG; break;
        default:
        fatald("Illegal type, token", Token.token);
    }

    // scan in one or more further * tokens
    // and determine the correct pointer type
    while(1) {
        scan(&Token);
        if (Token.token != T_STAR) break;
        type = pointer_to(type);
    }

    // here we leave the next token already scanned
    return type;

}

// Parse the declaration of variable
// It must be the 'int' token followed by an identifier 
// and a semicolon. Add the identifier to symbol table
void var_declaration(void) {
    int id, type;

    type = parse_type();
    matchident();
    id = addglob(Text,type, S_VARIABLE, 0);
    genglobsym(id);
    matchsemi();
}

// Parse the declaration of a simplistic function
struct ASTnode *function_declaration(void) {
    struct ASTnode *tree, *finalstmt;
    int nameslot, type, endlabel;

    // Get the type of the variable, then the identifier
    type = parse_type();
    matchident();
    
    // Get a label id for the end label (which can be jumped to)
    // Add the function to the symbol table
    // and set the Functional global to the function's symbol id
    endlabel = genlabel();
    nameslot = addglob(Text, type, S_FUNCTION, endlabel);
    Functionid = nameslot;

    matchlparen();
    matchrparen();

    // Get the AST for the body
    tree = compound_statement();

    // If the function type is not P_VOID, check that
    // the last AST operation in the compound statement was a return
    // statement
    if (type != P_VOID) {
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN)
            fatal("No return for function with non-void type");
    }

    // Return an A_FUNCTION node which has the function's nameslot
    // and the compound statement sub-tree
    return mkastunary(A_FUNCTION, P_VOID, tree, nameslot);
}