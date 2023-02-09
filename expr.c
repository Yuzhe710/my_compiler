#include "defs.h"
#include "scan.h"
#include "decl.h"

// parse the first factor and retrun an AST node representing it
static struct ASTnode *getleft(void) {
    struct ASTnode *n;
    int id;
    // For an INT node, make a leaf AST node for it and 
    // scan in the next token, otherwise, generate a syntax error
    switch (Token.token) {
        case T_INTLIT:
            n = mkastleaf(A_INTLIT, Token.intvalue);
            break;
        case T_IDENT:
            // check that this identifier exists
            id = findglob(Text);
            if (id == -1)
                fatals("Unknown variable", Text);
            //printf("%s\n", Gsym[id]->name);
            // Make a leaf AST node for it
            n = mkastleaf(A_IDENT, id);
            break;
        default:
            fatald("Syntax error, token", Token.token);
    }
    // scan the next token and return the leaf node
    scan(&Token);

    return n;
}

// convert a binary operator token into an AST operation
// an 1:1 mapping from token type to AST operation node type
int getoperation(int tokentype) {
    if (tokentype > T_EOF && tokentype < T_INTLIT) {
        return tokentype;
    }
    fatald("Syntax error, token", tokentype);
}

// Operator precedence for each token
static int OpPrec[] = {
    0, 10, 10,          // T_EOF, T_PLUS, T_MINUS
    20, 20,             // T_STAR, T_SLASH
    30, 30,             // T_EQ, T_NE
    40, 40, 40, 40      // T_LT, T_GT, T_LE, T_GE
    }; 

static int op_precedence(int tokentype) {
    int prec = OpPrec[tokentype];
    // printf("%d\n", tokentype);
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
    //printf("%d\n", Token.intvalue);
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


