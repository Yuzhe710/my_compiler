#include "defs.h"
#include "scan.h"
#include "decl.h"


int genAST(struct ASTnode *n) {
    int leftreg, rightreg;

    // Get the left and right sub-tree values
    if (n->left) 
        leftreg = genAST(n->left);
    if (n->right)
        rightreg = genAST(n->right);
    
    switch(n->op) {
        case A_ADD:
            return cgadd(leftreg, rightreg);
        case A_SUBSTRACT:
            return cgsub(leftreg, rightreg);
        case A_MULTIPLY:
            return cgmul(leftreg, rightreg);
        case A_DIVIDE:
            return cgdiv(leftreg, rightreg);
        case A_INTLIT:
            return cgload(n->intvalue);
        default:
            fprintf(stderr, "Unknown AST operator %d\n", n->op);
            exit(1);
    }
    
}

void genpreamble() {
    cgpreamble();
}

void genpostamble() {
  cgpostamble();
}
void genfreeregs() {
  freeall_registers();
}
void genprintint(int reg) {
  cgprintint(reg);
}

void generatecode(struct ASTnode *n) {
    int reg;

    cgpreamble();
    reg = genAST(n);
    cgprintint(reg);
    cgpostamble();
}