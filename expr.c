#include "defs.h"
#include "scan.h"
#include "decl.h"

// expression_list: <null>
//        | expression
//        | expression ',' expression_list
//        ;

// Parse a list of zero or more comma-seperated expressions and 
// return an AST composed of A_GLUE nodes with the left-hand child 
// being the sub-tree of previous expressions (or NULL) and the right-hand 
// child being the next expression. Each A_GLUE node will have the size field 
// set to the number of expressions in the tree at this point. If no expressions
// are parsed, NULL is returned.
//              A_FUNCCALL
//                  /
//               A_GLUE
//                /   \
//            A_GLUE  expr4
//             /   \
//         A_GLUE  expr3
//          /   \
//      A_GLUE  expr2
//      /    \
//    NULL  expr1
static struct ASTnode *expression_list(void) {
    struct ASTnode *tree = NULL;
    struct ASTnode *child = NULL;
    int exprcount = 0;

    // Loop until the final right parenthesis
    while (Token.token != T_RPAREN) {
        // Parse the next expression and increment the expression count
        child = binexpr(0);
        exprcount++;

        // Build an A_GLUE AST node with the previous tree as the left child
        // and the new expression as the right child. Store the expression count.
        tree = mkastnode(A_GLUE, P_NONE, tree, NULL, child, exprcount);

        // Must have a ',' or ')' at this point
        switch (Token.token) {
            case T_COMMA:
                scan(&Token);
                break;
            case T_RPAREN:
                break;
            default:
                fatald("Unexpected token in expression list", Token.token);
        }
    }
    // Return the tree of expressions
    return tree;
}

// Parse a function call with a single expression
// argument and return its AST
struct ASTnode *funccall(void) {
    struct ASTnode *tree;
    int id;

    // Check that the identifier has been defined
    // then make a leaf node for it.
    if ((id = findsymbol(Text)) == -1 || Symtable[id]->stype != S_FUNCTION) {
        fatals("Undeclared function", Text);
    }

    // Get the '('
    matchlparen();

    // Parse the following expression
    tree = expression_list();

    // XXX Check type of each argument against the function's prototype

    // Build the function call AST node. Store the function's
    // return type as this node's type.
    // Also record the function's symbol-id
    tree = mkastunary(A_FUNCCALL, Symtable[id]->type, tree, id);

    // Match the ')'
    matchrparen();
    return tree;
}

// Parse the index into an array and 
// return an AST tee for it
static struct ASTnode *array_access(void) {
    struct ASTnode *left, *right;
    int id;

    // Check 
    if ((id = findsymbol(Text)) == -1 || Symtable[id]->stype != S_ARRAY) {
        fatals("Undeclared array", Text);
    }
    left = mkastleaf(A_ADDR, Symtable[id]->type, id);

    // Get the '['
    scan(&Token);

    // Parse the following expression indicating index
    right = binexpr(0);

    // Get the ']'
    match(T_RBRACKET, "]");

    // Ensure that this is of int type
    if (!inttype(right->type))
        fatal("Array index is not of integer type");
    
    // Scale the index by the size of the element's type
    right = modify_type(right, left->type, A_ADD);

    // Return an AST tree where the array's base has the offset
    // added to it, and dereference the element.It is still an lvalue 
    left = mkastnode(A_ADD, Symtable[id]->type, left, NULL, right, 0);
    left = mkastunary(A_DEREF, value_at(left->type), left, 0);
    return left;
}


// parse the first factor and retrun an AST node representing it
static struct ASTnode *getleft(void) {
    struct ASTnode *n;
    int id;
    // For an INT node, make a leaf AST node for it and 
    // scan in the next token, otherwise, generate a syntax error
    switch (Token.token) {
        case T_STRLIT:
            // For a STRLIT token, generate the assembly for it.
            // Then make a leaf AST node for it. id is the string's label
            // make an anonymous global string. 
            // The label is effectively the base of the array of characters in the string
            id = genglobstr(Text);
            n = mkastleaf(A_STRLIT, P_CHARPTR, id);
            break;
        case T_INTLIT:
            // For an INTLIT token, make a leaf AST node for it
            // Make it a P_CHAR if it is within P_CHAR range (0..255)
            if (Token.intvalue >= 0 && Token.intvalue < 256)
                n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
            else    
                n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
            break;
        case T_IDENT:
            return postfix();
        case T_LPAREN:
            // Begining of a parenthesised expression
            scan(&Token);
            n = binexpr(0);
            matchrparen();

            return n;
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
    if (tokentype > T_EOF && tokentype <= T_SLASH) {
        return tokentype;
    }
    fatald("Syntax error, token", tokentype);
    return 0;
}

// Operator precedence for each token
static int OpPrec[] = {
    0, 10, 20, 30,      // T_EOF, T_ASSIGN, T_LOGOR, T_LOGAND
    40, 50, 60,         // T_OR, T_XOR, T_AMPER
    70, 70,             // T_EQ, T_NE
    80, 80, 80, 80,     // T_LT, T_GT, T_LE, T_GE
    90, 90,             // T_LSHIFT, T_RSHIFT
    100, 100,           // T_PLUS, T_MINUS
    110, 110            // T_STAR, T_SLASH
    }; 

static int op_precedence(int tokentype) {
    if (tokentype >= T_SLASH)
        fatald("Token with no precedence in op_precedence:", tokentype);
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
        
        case T_MINUS:
            // Get the next token and parse it
            // recursively as a prefix expression
            scan(&Token);
            tree = prefix();

            // Prepend a A_NEGATE operation to the tree and
            // make the child an rvalue. Because chars are unsigned, also 
            // widen this to int so that it is signed. (since ++ must be an int)
            tree->rvalue = 1;
            tree = modify_type(tree, P_INT, 0);
            tree = mkastunary(A_NEGATE, tree->type, tree, 0);
            break;
        case T_INVERT:
            // Get the next token and parse it
            // recursively as a prefix expression
            scan(&Token);
            tree = prefix();

            // Prepend a A_INVERT operation to the tree and 
            // make the child as rvalue. As invert must be an rvalue
            tree->rvalue = 1;
            tree = mkastunary(A_INVERT, tree->type, tree, 0);
            break;
        case T_LOGNOT:
            // Get the next token and parse it
            // recursively as a prefix expression
            scan(&Token);
            tree = prefix();

            // Prepend a A_LOGNOT operation to the tree and 
            // make the child an rvalue (as not must apply on rvalue)
            tree->rvalue = 1;
            tree = mkastunary(A_LOGNOT, tree->type, tree, 0);
            break;
        case T_INC:
            // Get the next token and parse it
            // recursively as a prefix expression
            scan(&Token);
            tree = prefix();

            // for now, ensure it is an identifier
            if (tree->op != A_IDENT)
                fatal("++ operator must be followed by an identifier");
            // Prepend an A_PREINC operation to the tree
            tree = mkastunary(A_PREINC, tree->type, tree, 0);
            break;
        case T_DEC:
            // Get the next token and parse it
            // recursively as a prefix expression
            scan(&Token);
            tree = prefix();

            // For now, ensure it is an identifier
            if (tree->op != A_IDENT)
                fatal("-- operator must be followed by an identifier");
            // Prepend an A_PREINC operation to the tree
            tree = mkastunary(A_PREDEC, tree->type, tree, 0);
            break;
        default:
            tree = getleft();
    }
    return tree;
}

// Parse a postfix expression and return
// an AST node representing it. The identifier is 
// already in Text
struct ASTnode *postfix(void) {
    struct ASTnode *n;
    int id;

    // Scan in the next token to see if we have a postfix expression
    scan(&Token);

    // Function call
    if (Token.token == T_LPAREN)
        return funccall();
    
    // An array reference
    if (Token.token == T_LBRACKET)
        return array_access();

    // A variable. Check that the variable exists
    id = findsymbol(Text);
    if (id == -1 || Symtable[id]->stype != S_VARIABLE)
        fatals("Unknown variable", Text);

    switch (Token.token) {
        // Post-increment: skip over the token
        case T_INC:
            scan(&Token);
            n = mkastleaf(A_POSTINC, Symtable[id]->type, id);
            break;
        case T_DEC:
            scan(&Token);
            n = mkastleaf(A_POSTDEC, Symtable[id]->type, id);
            break;
        // Just a variable reference
        default:
            n = mkastleaf(A_IDENT, Symtable[id]->type, id);
        
    }
    return n;
}


// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence
struct ASTnode *binexpr(int ptp) {
    struct ASTnode *left, *right;
    struct ASTnode *ltemp, *rtemp;
    int ASTop;
    int tokentype;
    
    left = prefix();
    // if hit a semi colon or ')', just return the left node
    tokentype = Token.token;
    if (tokentype == T_SEMI || tokentype == T_RPAREN || tokentype == T_RBRACKET || tokentype == T_COMMA) {
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
        // printf("ASTop is %d\n", ASTop);
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
        if (tokentype == T_SEMI || tokentype == T_RPAREN
            || tokentype == T_RBRACKET || tokentype == T_COMMA) {
            left->rvalue = 1;
            return left;
        }
    }
    // return the tree when its precedence is same or lower
    left->rvalue = 1;
    return left;
}


