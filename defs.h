#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TEXTLEN     512 


// represent "+", "-", "x", "/" and INT in current expressions
// namely token types
enum {
    T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INT, T_SEMI, T_PRINT
};

// token structure
struct token {
    int token;
    int intvalue;
};

//AST (abstract syntax tree) node types
enum {
    A_ADD, A_SUBSTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT
};

// AST structure
struct ASTnode {
    int op;     // operation to be performed
    struct ASTnode *left;       // left and right child trees
    struct ASTnode *right;
    int intvalue;               // for A_INTLIT, the integer value

};