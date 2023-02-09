#include "defs.h"
#include "scan.h"
#include "decl.h"




int genAST(struct ASTnode *n, int reg) {
    int leftreg, rightreg;

    // Get the left and right sub-tree values
    if (n->left) 
        leftreg = genAST(n->left, -1); // the left sub-tree will only be assignments and evaluated first, so no registers containg previous result is need
    if (n->right)
        rightreg = genAST(n->right, leftreg); // the right sub-tree will be lvalue (identifier), so register containing left expression's result is need
    
    switch(n->op) {
        case A_ADD:
            return cgadd(leftreg, rightreg);
        case A_SUBSTRACT:
            return cgsub(leftreg, rightreg);
        case A_MULTIPLY:
            return cgmul(leftreg, rightreg);
        case A_DIVIDE:
            return cgdiv(leftreg, rightreg);
        case A_EQ:
            return cgequal(leftreg, rightreg);
        case A_NE:
            return cgnotequal(leftreg, rightreg);
        case A_LT:
            return cglessthan(leftreg, rightreg);
        case A_GT:
            return cggreaterthan(leftreg, rightreg);
        case A_LE:
            return cglessequal(leftreg, rightreg);
        case A_GE:
            return cggreaterequal(leftreg, rightreg);
        case A_INTLIT:
            return cgloadint(n->v.intvalue);
        case A_IDENT:
            return cgloadglob(Gsym[n->v.id]->name);
        case A_LVIDENT:
            return cgstorglob(reg, Gsym[n->v.id]->name);
        case A_ASSIGN:
            return rightreg;
        
        
        default:
            fatald("Unknown AST operator", n->op);
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

void genglobsym(char *s) {
    cgglobsym(s);
}

// void generatecode(struct ASTnode *n) {
//     int reg;

//     cgpreamble();
//     reg = genAST(n);
//     cgprintint(reg);
//     cgpostamble();
// }