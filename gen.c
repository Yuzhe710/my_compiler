#include "defs.h"
#include "scan.h"
#include "decl.h"

// generate and return a new label number
static int label(void) {
    static int id = 1;
    return (id++);
}

// Generate assembly for an IF statement and its optional ELSE clause
static int genIFAST(struct ASTnode *n) {
    // if-else will need a label for false part and a label to jump to after the true
    // part
    int Lfalse, Lend;

    // Generate Label for false compound statement
    // and label for the end of overall IF statement (if exist)
    Lfalse = label();
    if (n->right) 
        Lend = label();
    
    // generate condition code 
    // followed by a zero jump to the false label
    // Here cheat by sending the Lfalse label as a register
    genAST(n->left, Lfalse, n->op);
    genfreeregs();

    // generate the true compound statement
    genAST(n->mid, NOREG, n->op);
    genfreeregs();

    // If there is an optional ELSE clause,
    // generate the jump to skip to the end
    if (n->right)
        cgjump(Lend);
    
    // False Label:
    cglabel(Lfalse);

    // Optional ELSE clause: generate
    // false compound statement and the 
    // end label
    if (n->right) {
        genAST(n->right, NOREG, n->op);
        genfreeregs();
        cglabel(Lend);
    }

    return NOREG;
}

// Generate code for WHILE statement
static int genWHILE(struct ASTnode *n) {
    int Lstart, Lend;

    // Generate the start and end label
    // and output the start label
    Lstart = label();
    Lend = label();
    cglabel(Lstart);

    // Generate the condition code
    // and a jump to the end label
    // We cheat by sending the Lfalse label as a register
    genAST(n->left, Lend, n->op);
    genfreeregs();

    // Generate the compound statement for the body
    genAST(n->right, NOREG, n->op);
    genfreeregs();

    // Finally output the jump back to the condition
    // and the end label
    cgjump(Lstart);
    cglabel(Lend);
    return NOREG;

}

// Given an AST, the register that holds the previous 
// rvalue, and the AST op of the parent, generate assembly code recursively
// return the register id containing tree's final value
int genAST(struct ASTnode *n, int reg, int parentASTop) {
    int leftreg, rightreg;

    // We have specific AST node handling at the top
    switch(n->op) {
        case A_IF:
            return genIFAST(n);
        case A_WHILE:
            return genWHILE(n);
        case A_GLUE:
            // generate for eah child,
            // and free registers after each child
            genAST(n->left, NOREG, n->op);
            genfreeregs();
            genAST(n->right, NOREG, n->op);
            genfreeregs();
            return NOREG;
        case A_FUNCTION:
            // Generate the function's preamble before the code
            cgfuncpreamble(Gsym[n->v.id]->name);
            genAST(n->left, NOREG, n->op);
            cgfuncpostamble();
            return NOREG;
    }

    //General AST node handling below

    // Get the left and right sub-tree values
    if (n->left) 
        leftreg = genAST(n->left, NOREG, n->op); // the left sub-tree will only be assignments and evaluated first, so no registers containg previous result is need
    if (n->right)
        rightreg = genAST(n->right, leftreg, n->op); // the right sub-tree will be lvalue (identifier), so register containing left expression's result is need
    
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
        case A_NE:
        case A_LT:
        case A_GT:
        case A_LE:
        case A_GE:
            // If the parent AST node is an A_IF
            // generate a compare followed by a jump
            // Otherwise, compare registers and set one register to 0 or 1 
            // based on the comparison
            if (parentASTop == A_IF || parentASTop == A_WHILE)
                return cgcompare_and_jump(n->op, leftreg, rightreg, reg);
            else 
                return cgcompare_and_set(n->op, leftreg, rightreg);
        case A_INTLIT:
            return cgloadint(n->v.intvalue);
        case A_IDENT:
            return cgloadglob(Gsym[n->v.id]->name);
        case A_LVIDENT:
            return cgstorglob(reg, Gsym[n->v.id]->name);
        case A_ASSIGN:
            return rightreg;
        case A_PRINT:
            // Print the left child's value, 
            // and return no register
            genprintint(leftreg);
            genfreeregs();
            return NOREG;
        
        default:
            fatald("Unknown AST operator", n->op);
    }
    
}

void genpreamble() {
    cgpreamble();
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