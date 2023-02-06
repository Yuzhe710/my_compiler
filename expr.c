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

// convert a binary operator token into an AST operation
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

// Operator precedence for each token
static int OpPrec[] = {0, 10, 10, 20, 20, 0}; // EOF, +, -, *, /, Int

static int op_precedence(int tokentype) {
    int prec = OpPrec[tokentype];
    if (prec == 0) {
        fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);
        exit(1);
    }
    return prec;
}



// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence
struct ASTnode *binexpr(int ptp) {
    struct ASTnode *left, *right;

    int tokentype;

    // Get the integer literal on the left
    // get the next token at the same time
    left = getleft();

    // if hit a semi colon, just return the left node
    tokentype = Token.token;
    if (tokentype == T_SEMI)
        return left;

    // while the precedence of this token is 
    // more than that of the previous token precedence
    while (op_precedence(tokentype) > ptp) {
        // get the integer literal
        scan(&Token);

        // recursively call binexpr() with 
        // precedence of our token to build a sub-tree
        right = binexpr(OpPrec[tokentype]);

        // Join that sub-tree with ours
        left = mkastnode(getoperation(tokentype), left, right, 0);

        // update the tokentype to be the type of current token
        // If we hit a semi-colon, just return the left node
        tokentype = Token.token;
        if (tokentype == T_SEMI) 
            return left;
    }
    // return the tree when its precedence is same or lower
    return left;
}


