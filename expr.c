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
        case T_LPAREN:
            // Begining of a parenthesised expression
            scan(&Token);
            n = binexpr(0);
            matchrparen();

            return n;
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
    0, 10,              // T_EOF, T_ASSIGN
    20, 20,             // T_PLUS, T_MINUS
    30, 30,             // T_STAR, T_SLASH
    40, 40,             // T_EQ, T_NE
    50, 50, 50, 50      // T_LT, T_GT, T_LE, T_GE
    }; 

static int op_precedence(int tokentype) {
    int prec = OpPrec[tokentype];
    // printf("%d\n", tokentype);
    if (prec == 0) {
        fatald("Syntax error, token", tokentype);
    }
    return prec;
}

static int rightassoc(int tokentype) {
    if (tokentype == T_ASSIGN)
        return 1;
    return 0;
}

// Parse a prefix expression and return
// a sub-tree representing it
// currently only support one * or & after an identifier
struct ASTnode *prefix(void) {
    struct ASTnode *tree;
    switch (Token.token) {
        case T_AMPER:
            // Get the next token and parse it recursively
            // as a prefix expression
            scan(&Token);

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
            tree = getleft();
    }
    return tree;
}



// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence
struct ASTnode *binexpr(int ptp) {
    struct ASTnode *left, *right;
    struct ASTnode *ltemp, *rtemp;
    int ASTop;
    int tokentype;
    //printf("%d\n", Token.intvalue);
    // Get the integer literal on the left
    // get the next token at the same time
    left = prefix();

    // if hit a semi colon or ')', just return the left node
    tokentype = Token.token;
    if (tokentype == T_SEMI || tokentype == T_RPAREN) {
        left->rvalue = 1;
        return left;
    }
    // while the precedence of this token is 
    // more than that of the previous token precedence
    while (op_precedence(tokentype) > ptp ||
            (rightassoc(tokentype) && op_precedence(tokentype) == ptp)) {
        // get the integer literal
        scan(&Token);

        // recursively call binexpr() with 
        // precedence of our token to build a sub-tree
        right = binexpr(OpPrec[tokentype]);
        // Ensure the two types are compatible by trying
        // to modify each tree to match the other's type.
        ASTop = getoperation(tokentype);

        if (ASTop == A_ASSIGN) {
            // Assignment
            // Make the right tree into an rvalue
            right->rvalue = 1;

            // Ensure the right's type matches the left
            right = modify_type(right, left->type, 0);
            if (left == NULL)
                fatal("Incompatible expression in assignment");

            // Make an assignment AST tree. 
            // But switch left and right around.
            // e.g.
            //     =
            //    / \ 
            //   a   3
            // 
            // will become 
            //     =
            //    / \
            //   3   a
            //
            //  a = b = 3
            //         =
            //        / \
            //       =   a
            //      / \
            //     3   b
            ltemp = left; left = right; right = ltemp;
        } else {

            // We are not doing an assignment, so both trees should be rvalues
            // Convert both trees into rvalue if they are lvalue trees. (lavlue is default)
            left->rvalue = 1;
            right->rvalue = 1;

            // Ensure the two types are compatible by trying 
            // to modify each tree to match the other's type
            ltemp = modify_type(left, right->type, ASTop);
            rtemp = modify_type(right, left->type, ASTop);
            if (ltemp == NULL && rtemp == NULL)
                fatal("Incompatible types in binary expression");
            if (ltemp != NULL)
                left = ltemp;
            if (rtemp != NULL)
                right = rtemp;
        }
        
        
        // Join that sub-tree with ours
        left = mkastnode(getoperation(tokentype), left->type, left, NULL, right, 0);

        // update the tokentype to be the type of current token
        // If we hit a semi-colon, just return the left node
        tokentype = Token.token;
        if (tokentype == T_SEMI || tokentype == T_RPAREN) {
            left->rvalue = 1;
            return left;
        }
    }
    // return the tree when its precedence is same or lower
    left->rvalue = 1;
    return left;
}


