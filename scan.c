#include "defs.h"
#include "scan.h"
#include "decl.h"



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

// scan the identifier 
static int scanindentifier(int c, char *buf, int lim) {
    int i = 0;

    // identifier could contain alphabet, digit or underscore
    while (isalpha(c) || isdigit(c) || c == '_') {
        if (i == lim - 1) {
            printf("identifier too long on line %d\n", Line);
            exit(1);
        } else if (i < lim - 1) {
            buf[i++] = c;
        }
        c = next();
    }

    // hit a non-valid character, put it back
    Putback = c;
    buf[i] = '\0';
    return i;
}

// Given a word from the input, return the matching 
// keyword token number or 0 if not a keyword.
// It is firstly compare the first char, so we do not 
// have to strcmp with all keywords
static int keyword(char *s) {
    switch(*s) {
        case 'p':
            if (!strcmp(s, "print")) {
                return (T_PRINT);
            }

            break;
    }
    return 0;
}

int scan(struct token *t) {
    int c, tokentype;

    // skip
    c = skip();

    // Determine the token based on input char
    switch(c) {
        case EOF:
            t->token = T_EOF;
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
        case ';':
            t->token = T_SEMI;
            break;
        default:
            // if it is a digit, scanint
            if (isdigit(c)) {
                t->intvalue = scanint(c);
                t->token = T_INT;
                break;
            } else if (isalpha(c) || c == '_') {
                // read a keyword or identifier
                scanindentifier(c, Text, TEXTLEN);
                if (tokentype = keyword(Text)) {
                    t->token = tokentype;
                    break;
                }
                // Not a recognised keyword, so an error for now
                printf("Unrecognised symbol %s on line %d\n", Text, Line);
                exit(1);
            }

            printf("Unrecognised character %c on line %d\n", c, Line);
            exit(1);
    }

    // a token is found
    return 1;
}

