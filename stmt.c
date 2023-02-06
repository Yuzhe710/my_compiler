#include "defs.h"
#include "scan.h"
#include "decl.h"
// parse all the main statements in the language
// parse one or more statements
void statements(void) {
    struct ASTnode *tree;
    int reg;

    while (1) {
        // Match a print as first token
        match(T_PRINT, "print");

        // Parse following expression and generate assembly code
        tree = binexpr(0);
        reg = genAST(tree);
        genprintint(reg);
        genfreeregs();

        // match the following semicolon
        // and stop if we are at EOF
        matchsemi();
        if (Token.token == T_EOF)
            return;
    }
}