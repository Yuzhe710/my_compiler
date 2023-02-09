#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TEXTLEN     512     // Length of symbols in input
#define NSYMBOLS    1024    // Number of symbol table entries

// represent "+", "-", "x", "/" and INT in current expressions
// namely token types
enum {
    T_EOF, 
    T_PLUS, T_MINUS, 
    T_STAR, T_SLASH, 
    T_EQ, T_NE,
    T_LT, T_GT, T_LE, T_GE,
    T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,

    T_PRINT, T_INT
};

// token structure
struct token {
    int token;
    int intvalue;
};

//AST (abstract syntax tree) node types
enum {
    A_ADD, A_SUBSTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT,
    A_IDENT, A_LVIDENT, A_ASSIGN
};

// AST structure
struct ASTnode {
    int op;                     // operation to be performed
    struct ASTnode *left;       // left and right child trees
    struct ASTnode *right;
    int intvalue;               
    union {
        int intvalue;           // for A_INTLIT, the integer value
        int id;                 // for A_IDENT, the symbol slot number
    } v;
};

struct symtable {
    char *name;      // Name of a symbol
};