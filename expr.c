#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse a function call with a single expression
// argument and return its AST
struct ASTnode *funccall(void) {
    struct ASTnode *tree;
    int id;

    // Check that the identifier has been defined
    // then make a leaf node for it.
    if ((id = findglob(Text)) == -1) {
        fatals("Undeclared function", Text);
    }

    // Get the '('
    matchlparen();

    // Parse the following expression
    tree = binexpr(0);

    // Build the function call AST node. Store the function's
    // return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, Gsym[id]->type, tree, id);

    // Match the ')'
    matchrparen();
    return tree;
}


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
            // This could be a variable or a function call
            scan(&Token);

            // It's a '(', so a function call
            if (Token.token == T_LPAREN)
                return funccall();

            // Not a function call, so reject the new token
            reject_token(&Token);

            // check that this identifier exists
            id = findglob(Text);
            if (id == -1)
                fatals("Unknown variable", Text);
            //printf("%s\n", Gsym[id]->name);
            // Make a leaf AST node for it
            n = mkastleaf(A_IDENT, Gsym[id]->type, id);
            break;
        default:
            // printf("here is %d\n", Token.token);
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
    return 0;
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

// Parse a prefix expression and return
// a sub-tree representing it
// currently only support one * or & after an identifier
struct ASTnode *prefix(void) {
    struct ASTnode *tree;
    switch (Token.token) {
        case T_AMPER:
            // printf("Token this place is %d\n", Token.token);
            // Get the next token and parse it recursively
            // as a prefix expression
            scan(&Token);
            //printf("Token next is %d\n", Token.token);
            tree = prefix();

            // Ensure the next token is an identifier
            if (tree->op != A_IDENT) 
                fatal("& operator must be followed by an identifier");
            
            // Change the operator to A_ADDR and the type to 
            // a pointer to the original type
            tree->op = A_ADDR;
            tree->type = pointer_to(tree->type);
            break;
        case T_STAR:
            // Get the next token and parse it recursively
            // as a prefix expression
            // printf("Token *\n");
            scan(&Token);
            tree = prefix();

            // For now, ensure the next token is either another deference
            // or identifier
            if (tree->op != A_IDENT && tree->op != A_DEREF)
                fatal("* operator must be followed by an identifier or *");

            // Prepend an A_DEREF operation to the tree
            tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);
            break;
        default:
            // printf("Token here is %d\n", Token.token);
            tree = getleft();
    }
    return tree;
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
    left = prefix();

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


