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
        case 'i':
            if (!strcmp(s, "int")) {
                return T_INT;
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
        case '=':
            if ((c = next()) == '=') {
                t->token = T_EQ;
            } else {
                Putback = c;
                t->token = T_ASSIGN;
            }
            break;
        case '!':
            if ((c = next()) == '=') {
                t->token = T_NE;
            } else {
                fatalc("Unrecognised character", c);
            }
            break;
        case '<':
            if ((c = next()) == '=') {
                t->token = T_LE;
            } else {
                Putback = c;
                t->token = T_LT;
            }
            break;
        case '>':
            if ((c = next()) == '=') {
                t->token = T_GE;
            } else {
                Putback = c;
                t->token = T_GT;
            }
            break;
        default:
            // if it is a digit, scanint
            if (isdigit(c)) {
                t->intvalue = scanint(c);
                t->token = T_INTLIT;
                break;
            } else if (isalpha(c) || c == '_') {
                // read a keyword or identifier
                scanindentifier(c, Text, TEXTLEN);

                // if it is a recognised token, return that token
                if (tokentype = keyword(Text)) {
                    t->token = tokentype;
                    break;
                }
                // Not a recognised keyword, so it must be an identifier
                t->token = T_IDENT;
                break;
            }
            // The character isn't part of any recognised token, throw error
            fatalc("Unrecognised character", c);
    }

    // a token is found
    return 1;
}

