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
            // For an INTLIT token, make a leaf AST node for it
            // Make it a P_CHAR if it is within P_CHAR range (0..255)
            if (Token.intvalue >= 0 && Token.intvalue < 256)
                n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
            else    
                n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
            break;
        case T_IDENT:
            // check that this identifier exists
            id = findglob(Text);
            if (id == -1)
                fatals("Unknown variable", Text);
            //printf("%s\n", Gsym[id]->name);
            // Make a leaf AST node for it
            n = mkastleaf(A_IDENT, Gsym[id]->type, id);
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
        fatald("Syntax error, token", tokentype);
    }
    return prec;
}



// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence
struct ASTnode *binexpr(int ptp) {
    struct ASTnode *left, *right;
    int lefttype, righttype;
    int tokentype;
    //printf("%d\n", Token.intvalue);
    // Get the integer literal on the left
    // get the next token at the same time
    left = getleft();

    // if hit a semi colon, just return the left node
    tokentype = Token.token;
    if (tokentype == T_SEMI || tokentype == T_RPAREN)
        return left;

    // while the precedence of this token is 
    // more than that of the previous token precedence
    while (op_precedence(tokentype) > ptp) {
        // get the integer literal
        scan(&Token);

        // recursively call binexpr() with 
        // precedence of our token to build a sub-tree
        right = binexpr(OpPrec[tokentype]);

        // ensure the two types are compatible
        lefttype = left->type;
        righttype = right->type;
        if (!type_compatible(&lefttype, &righttype, 0)) 
            fatal("Incompatible types");
        
        // Widen either side if required, type vars are A_WIDEN now
        if (lefttype)
            left = mkastunary(lefttype, right->type, left, 0);
        if (righttype)
            right = mkastunary(righttype, left->type, right, 0);
        
        // Join that sub-tree with ours
        left = mkastnode(getoperation(tokentype), left->type, left, NULL, right, 0);

        // update the tokentype to be the type of current token
        // If we hit a semi-colon, just return the left node
        tokentype = Token.token;
        if (tokentype == T_SEMI || tokentype == T_RPAREN) 
            return left;
    }
    // return the tree when its precedence is same or lower
    return left;
}


