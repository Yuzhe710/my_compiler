#include "defs.h"
#include "scan.h"
#include "decl.h"

// List of AST operators
static char *ASTop[] = { "+", "-", "*", "/" };

// Given an AST, interpret operations and return a final value
int interpretAST(struct ASTnode *n) {
    int leftval, rightval;

    // Get the left and right sub-tree
    if (n->left)
        leftval = interpretAST(n->left);
    if (n->right)
        rightval = interpretAST(n->right);

    // Debug: Print what we are about to do
    // if (n->op == A_INTLIT)
    //     printf("int %d\n", n->intvalue);
    // else
    //     printf("%d %s %d\n", leftval, ASTop[n->op], rightval);

    
    switch(n->op) {
        case A_ADD:
            return (leftval + rightval);
        case A_SUBSTRACT:
            return (leftval - rightval);
        case A_MULTIPLY:
            return (leftval * rightval);
        case A_DIVIDE:
            return (leftval / rightval);
        case A_INTLIT:
            return n->intvalue;
        default:
            fprintf(stderr, "Unknown AST operator %d\n", n->op);
            exit(1);
    }
}