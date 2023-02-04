#include "defs.h"
#include "scan.h"
#include "decl.h"

// parse the first factor and retrun an AST node representing it
static struct ASTnode *getleft(void) {
    struct ASTnode *n;
    
    // For an INT node, make a leaf AST node for it and 
    // scan in the next token, otherwise, generate a syntax error
    switch (Token.token) {
        case T_INT:
            n = mkastleaf(A_INTLIT, Token.intvalue);
            scan(&Token);
            return n;
        default:
            fprintf(stderr, "syntax error on line %d\n", Line);
            exit(1);
    }
}

int getoperation(int tok) {
    switch (tok) {
        case T_PLUS:
            return (A_ADD);
        case T_MINUS:
            return (A_SUBSTRACT);
        case T_STAR:
            return (A_MULTIPLY);
        case T_SLASH:
            return (A_DIVIDE);
        default:
            fprintf(stderr, "unknown token in arithop() on line %d\n", Line);
            exit(1);
    }
}

// Return an AST tree whose root is a binary operator
struct ASTnode *binexpr(void) {
    struct ASTnode *n, *left, *right;
    int nodetype;

    // Get the integer literal on the left
    // get the next token at the same time
    left = getleft();

    // if no token left, just return the left node
    if (Token.token == T_EOF)
        return left;

    // convert the token into a node type
    nodetype = getoperation(Token.token);

    // get the next token
    scan(&Token);

    // recursively get the right hand tree
    right = binexpr();

    // build the tree woth both sub-trees
    n = mkastnode(nodetype, left, right, 0);

    return n;
}


