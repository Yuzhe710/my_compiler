#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// represent "+", "-", "x", "/" and INT in current expressions
enum {
    T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INT
};

// token structure
struct token {
    int token;
    int intvalue;
};