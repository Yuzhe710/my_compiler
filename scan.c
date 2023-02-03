#include "defs.h"
#include "decl.h"

int Line = 1;
int Putback = '\n';
FILE *Infile;

// return the next char from input, which could be the Putback character
static int next(void) {

    int c;

    if (Putback) {
        c = Putback;
        Putback = 0;
        return c;
    }

    c = fgetc(Infile);
    if (c == '\n')
        Line ++;
    
    return c;
}

// skip the next char which do not need to deal with
// i.e. whitespace, newlines, carriage return, tab, form feed 
static int skip(void) {

    int c;

    c = next();

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
        c = next();
    }
    return c;
}

static int getcharpos(char *s, int c) {
    char *p;
    p = strchr(s, c);
    return (p ? p - s : -1);
}

// scan and return an integer literal
static int scanint(int c) {
    int k, val = 0;

    while ((k = getcharpos("0123456789", c)) >= 0) {
        val = val * 10 + k;
        c = next();
    }

    // hit a non-integer char, put it back
    Putback = c;
    return val;
}


int scan(struct token *t) {
    int c;

    // skip
    c = skip();

    // Determine the token based on input char
    switch(c) {
        case EOF:
            return 0;
        case '+':
            t->token = T_PLUS;
            break;
        case '-':
            t->token = T_MINUS;
            break;
        case '*':
            t->token = T_STAR;
            break;
        case '/':
            t->token = T_SLASH;
            break;
        default:
            // if it is a digit, scanint
            if (isdigit(c)) {
                t->intvalue = scanint(c);
                t->token = T_INT;
                break;
            }

            printf("Unrecognised character %c on line %d\n", c, Line);
            exit(1);
    }

    // a token is found
    return 1;
}

