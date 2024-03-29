#include "defs.h"
#include "scan.h"
#include "decl.h"

// generate and return a new label number
int genlabel(void) {
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
    Lfalse = genlabel();
    if (n->right) 
        Lend = genlabel();
    
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
    Lstart = genlabel();
    Lend = genlabel();
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

// Generate the code to copy the arguments of a function call
// to its parameters, then call the function itself. Return 
// the register that holds the function's return value.
static int gen_funccall(struct ASTnode *n) {
    struct ASTnode *gluetree = n->left;
    int reg;
    int numargs = 0;

    // If there is a list of arguments, walk this list
    // from the last argument (right-hand child) to the 
    // first
    while (gluetree) {
        // Calculate the expression's value
        reg = genAST(gluetree->right, NOLABEL, gluetree->op);
        // Copy this into the n'th function parameter: size is 1, 2, 3, ...
        cgcopyarg(reg, gluetree->v.size);
        // Keep the first (highest) number of arguments
        if (numargs == 0) numargs = gluetree->v.size;
        genfreeregs();
        gluetree = gluetree->left;
    }

    // Call the function, clean up the stack (based on numargs),
    // and return its result
    return cgcall(n->sym, numargs);

}

// Given an AST, the register that holds the previous 
// rvalue, and the AST op of the parent, generate assembly code recursively
// return the register id containing tree's final value
int genAST(struct ASTnode *n, int label, int parentASTop) {
    int leftreg, rightreg;

    // We have specific AST node handling at the top
    switch(n->op) {
        case A_IF:
            return genIFAST(n);
        case A_WHILE:
            return genWHILE(n);
        case A_FUNCCALL:
            return gen_funccall(n);
        case A_GLUE:
            // generate for eah child,
            // and free registers after each child
            genAST(n->left, NOLABEL, n->op);
            genfreeregs();
            genAST(n->right, NOLABEL, n->op);
            genfreeregs();
            return NOREG;
        case A_FUNCTION:
            // Generate the function's preamble before the code
            cgfuncpreamble(n->sym);
            genAST(n->left, NOLABEL, n->op);
            cgfuncpostamble(n->sym);
            return NOREG;
    }

    //General AST node handling below

    // Get the left and right sub-tree values
    if (n->left) 
        leftreg = genAST(n->left, NOLABEL, n->op); // the left sub-tree will only be assignments and evaluated first, so no registers containg previous result is need
    if (n->right)
        rightreg = genAST(n->right, NOLABEL, n->op); // the right sub-tree will be lvalue (identifier), so register containing left expression's result is need
    
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
                return cgcompare_and_jump(n->op, leftreg, rightreg, label);
            else 
                return cgcompare_and_set(n->op, leftreg, rightreg);
        case A_INTLIT:
            return cgloadint(n->v.intvalue, n->type);
        case A_IDENT:
            // Load our value if we are an rvalue
            // or we are being dereferenced
            if (n->rvalue || parentASTop == A_DEREF) {
                if (n->sym->class == C_GLOBAL) {

                    return cgloadglob(n->sym, n->op);
                } else {
                    return cgloadlocal(n->sym, n->op);
                }
            } else {
                return NOREG;
            }
        case A_ASSIGN:
            // Are we assigning to an identifier or through a pointer?
            switch (n->right->op) {
                case A_IDENT: 
                    if (n->right->sym->class == C_GLOBAL)
                        return cgstorglob(leftreg, n->right->sym);
                    else {
                        return cgstorlocal(leftreg, n->right->sym);
                    }
                case A_DEREF: return cgstorderef(leftreg, rightreg, n->right->type);
                default: fatald("Can't A_ASSIGN in genAST(), op", n->op);
            }
        case A_WIDEN:
            // Widen the child's type to the parent's type
            return cgwiden(leftreg, n->left->type, n->type);
        case A_RETURN:
            cgreturn(leftreg, Functionid);
            return NOREG;
        case A_ADDR:
            return cgaddress(n->sym);
        case A_DEREF:
            // If we are an rvalue, dereference to get the value we point at
            // otherwise leave it for A_ASSIGN to store through the pointer
            if (n->rvalue)
                return cgderef(leftreg, n->left->type);
            else    
                return leftreg;
        case A_SCALE:
            // Optimisation trick: use shift 
            // if the scale value is a known power of 2
            switch (n->v.size) {
                case 2: return cgshlconst(leftreg, 1);
                case 4: return cgshlconst(leftreg, 2);
                case 8: return cgshlconst(leftreg, 3);
                default:
                    // Load a register with the size 
                    // and multiply the leftreg by this size
                    rightreg = cgloadint(n->v.size, P_INT);
                    return cgmul(leftreg, rightreg);
            }
        case A_STRLIT:
            return cgloadglobstr(n->v.intvalue);
        case A_AND:
            return (cgand(leftreg, rightreg));
        case A_OR:
            return (cgor(leftreg, rightreg));
        case A_XOR:
            return (cgxor(leftreg, rightreg));
        case A_LSHIFT:
            return (cgshl(leftreg, rightreg));
        case A_RSHIFT:
            return (cgshr(leftreg, rightreg));
        case A_POSTINC:
        case A_POSTDEC:
            // Load the variable's value into a register,
            // then decrement it
            if (n->sym->class == C_GLOBAL)
                return (cgloadglob(n->sym, n->op));
            else
                return cgloadlocal(n->sym, n->op);
        case A_PREINC:
        case A_PREDEC:
            // Load and decrement the variable's value into a register
            if (n->left->sym->class == C_GLOBAL)
                return (cgloadglob(n->left->sym, n->op));
            else 
                return (cgloadlocal(n->left->sym, n->op));
        case A_NEGATE:
            return (cgnegate(leftreg));
        case A_INVERT:
            return (cginvert(leftreg));
        case A_LOGNOT:
            return (cglognot(leftreg));
        case A_TOBOOL:
            // If the parent AST node is an A_IF or A_WHILE, generate
            // a compare followed by a jump. Otherwise, set the register
            // to 0 or 1 based on it's zeroeness or non-zeroeness
            return (cgboolean(leftreg, parentASTop, label));
        default:
            fatald("Unknown AST operator", n->op);
    }
    return (NOREG);
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

void genglobsym(struct symtable *node) {
    cgglobsym(node);
}

int genprimsize(int type) {
    return cgprimsize(type);
}

int genglobstr(char *strvalue) {
    int l = genlabel();
    cgglobstr(l, strvalue);
    return l;
}

void genpostamble() {
    cgpostamble();
}

// void generatecode(struct ASTnode *n) {
//     int reg;

//     cgpreamble();
//     reg = genAST(n);
//     cgprintint(reg);
//     cgpostamble();
// }