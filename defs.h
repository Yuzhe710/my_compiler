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
    // operators
    T_PLUS, T_MINUS, 
    T_STAR, T_SLASH, 
    T_EQ, T_NE,
    T_LT, T_GT, T_LE, T_GE,
    // Type keyword
    T_VOID, T_CHAR, T_INT, T_LONG, 
    // Structural tokens
    T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
    T_AMPER, T_LOGAND, T_COMMA,
    // other keywords
    T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN

};

// token structure
struct token {
    int token;
    int intvalue;
};

//AST (abstract syntax tree) node types
enum {
    A_ADD=1, A_SUBSTRACT, A_MULTIPLY, A_DIVIDE, 
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
    A_INTLIT,
    A_IDENT, A_LVIDENT, A_ASSIGN, A_PRINT,
    A_GLUE, A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
    A_FUNCCALL, A_DEREF, A_ADDR
};

// Primitives types
enum {
    P_NONE, P_VOID, P_CHAR, P_INT, P_LONG,
    P_VOIDPTR, P_CHARPTR, P_INTPTR, P_LONGPTR
};

// AST structure
struct ASTnode {
    int op;                     // operation to be performed
    int type;
    struct ASTnode *left;       // left, mid and right child trees
    struct ASTnode *mid;
    struct ASTnode *right;
    int intvalue;               
    union {
        int intvalue;           // for A_INTLIT, the integer value
        int id;                 // for A_IDENT, the symbol slot number
    } v;
};

#define NOREG -1                // Use NOREG when the AST 

// Structural types
enum {
    S_VARIABLE, S_FUNCTION
};

// Symbol table structure
struct symtable {
    char *name;      // Name of a symbol
    int type;        // Primitive type for the symbol
    int stype;       // Structural type for the symbol (future)
    int endlabel;    // For S_FUNCTIONs, the end label
};