#include "defs.h"
#include "scan.h"
#include "decl.h"

// parse one or more statements 
void statements(void) {

    while (1) {
        switch(Token.token) {
            case T_PRINT:
                print_statement();
                break;
            case T_INT:
                var_declaration();
                break;
            case T_IDENT:
                assignment_statement();
                break;
            case T_EOF:
                return;
            default:
                fatald("Syntax error, token", Token.token);
        }
    }
}

void print_statement(void) {
    struct ASTnode *tree;
    int reg;

    // Match a 'print' as the first token
    match(T_PRINT, "print");

    // Parse the following expression and
    // generate the assembly code
    tree = binexpr(0);
    reg = genAST(tree, -1);
    genprintint(reg);
    genfreeregs();

    // Match the following semicolon
    matchsemi();

}

// In assignment, identifier is the rvalue. the operation for the rvalue node is A_LVIDENT
void assignment_statement(void) {
    struct ASTnode *left, *right, *tree;
    int id;

    // Ensure we have an identifier
    matchident();

    // check if it has been defined and make leaf node for it
    if ((id = findglob(Text)) == -1) {
        fatals("Undeclared variable", Text);
    }
    right = mkastleaf(A_LVIDENT, id);

    match(T_EQUALS, "=");

    // Rvalue expression needs to be evaluated before saved to variable, make it left tree.
    left = binexpr(0);
    // printf("%d\n", left->v.intvalue);
    tree = mkastnode(A_ASSIGN, left, right, 0);

    genAST(tree, -1);
    genfreeregs();

    matchsemi();
}