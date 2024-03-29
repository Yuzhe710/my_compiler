#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TEXTLEN     512     // Length of symbols in input
#define NSYMBOLS    1024    // Number of symbol table entries

// Commands and defaults
#define AOUT "a.out"
#define ASCMD "as -o"
#define LDCMD "gcc -o"

// represent "+", "-", "x", "/" and INT in current expressions
// namely token types
enum {
    T_EOF, 
    // operators
    T_ASSIGN, T_LOGOR, T_LOGAND,
    T_OR, T_XOR, T_AMPER,
    T_EQ, T_NE,
    T_LT, T_GT, T_LE, T_GE,
    T_LSHIFT, T_RSHIFT,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, 
    
    // other operators
    T_INC, T_DEC, T_INVERT, T_LOGNOT,

    // Type keyword
    T_VOID, T_CHAR, T_INT, T_LONG, 
    // other keywords
    T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
    // Structural tokens
    T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
    T_LBRACKET, T_RBRACKET, T_COMMA
};

// token structure
struct token {
    int token;
    int intvalue;
};

//AST (abstract syntax tree) node types
enum {
    A_ASSIGN=1, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND,
    A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,
    A_ADD, A_SUBSTRACT, A_MULTIPLY, A_DIVIDE,
    A_INTLIT, A_STRLIT, A_IDENT, A_GLUE,
    A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN,
    A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,
    A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,
    A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL
};

// Primitives types
// The bottom 4 bits is an integer 
// value that represents the level of indirection
// e.g. 0= no pointer, 1= pointer, 2= pointer pointer ect.
enum {
    P_NONE, P_VOID=16, P_CHAR=32, P_INT=48, P_LONG=64
};

// AST structure
struct ASTnode {
    int op;                     // operation to be performed
    int type;
    int rvalue;                 // True if the node is a rvalue
    struct ASTnode *left;       // left, mid and right child trees
    struct ASTnode *mid;
    struct ASTnode *right;
    struct symtable *sym;       // For many AST nodes, the pointer to the symbol in symbol table
    // For A_INTLIT, the integer value
    // For A_IDENT, the symbol slot number
    // For A_FUNCTION, the symbol slot number
    // For A_SCALE, the size to scale by
    // For A_FUNCCALL, the 
    union {
        int intvalue;           // for A_INTLIT, the integer value
        int size;               // for A_SCALE, the symbol slot number
    } v;
};

enum {
    NOREG = -1,                 // Use NOREG when the AST generation function has no register to return
    NOLABEL = 0                 // Use NOLABEL when we have no label pass to genAST()
};

// Structural types
enum {
    S_VARIABLE, S_FUNCTION, S_ARRAY
};

// Storage class
enum {
    C_GLOBAL = 1,        // Globally visable symbol
    C_LOCAL,             // Locally visible symbol
    C_PARAM              // Locally visible function parameter
};



// Symbol table structure
struct symtable {
    char *name;      // Name of a symbol
    int type;        // Primitive type for the symbol
    int stype;       // Structural type for the symbol (future)
    int class;       // Storage class for the symbol
    union {
        int endlabel;    // For S_FUNCTIONs, the end label
        int size;        // Number of elements in the symbol
    };
    union {
        int posn;        // For locals, the negative offset from the stack base pointer
        int nelems;      // For Functions, the number of parameters
    };
    struct symtable *next;      // Next symbol in one list
    struct symtable *member;    // First member of a function, struct, union, or enum
};