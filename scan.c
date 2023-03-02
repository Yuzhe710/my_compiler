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
            fatal("Identifier too long");
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
        case 'c':
            if (!strcmp(s, "char"))
                return T_CHAR;
            break;
        case 'e':
            if (!strcmp(s, "else")) {
                return T_ELSE;
            }
            break;
        case 'f':
            if (!strcmp(s, "for")) {
                return T_FOR;
            }
            break;
        case 'i':
            if (!strcmp(s, "int")) {
                return T_INT;
            }
            if (!strcmp(s, "if")) {
                return T_IF;
            }
            break;
        case 'l':
            if (!strcmp(s, "long")) {
                return T_LONG;
            }
            break;
        case 'r':
            if (!strcmp(s, "return")) {
                return T_RETURN;
            }
            break;
        case 'v':
            if (!strcmp(s, "void")) {
                return T_VOID;
            }
            break;
        case 'w':
            if (!strcmp(s, "while")) {
                return T_WHILE;
            }
            break;
    }
    return 0;
}

// Return the next character from a character 
// or string literal
static int scanch(void) {
    int c;

    // Get the next input character and interpret
    // metacharacters that start with a blackslash
    c = next();
    if (c == '\\') {
        switch(c = next()) {
            case 'a': return '\a';
            case 'b':  return '\b';
            case 'f':  return '\f';
            case 'n':  return '\n';
            case 'r':  return '\r';
            case 't':  return '\t';
            case 'v':  return '\v';
            case '\\': return '\\';
            case '"':  return '"' ;
            case '\'': return '\'';
            default:
                fatalc("unknown escape sequence", c);
        }
    }
    return c;
}

// Scan in a string literal from the input file, 
// and store it in buf[]. Return the length of
// the string
static int scanstr(char *buf) {
    int i, c;

    // Loop while we have enough buffer space
    for (int i = 0; i < TEXTLEN-1; i++) {
        // Get the next char and append to buf
        // Return when we hit the ending double quote
        if ((c = scanch()) == '"') {
            buf[i] = 0;
            return i;
        }
        buf[i] = c;
    }
    // Ran out of buf[] space
    fatal("String literal too long");
    return(0);
}

// A pointer to a rejected token
static struct token *Rejtoken = NULL;

// reject the token we just scanned
void reject_token(struct token *t) {
    if (Rejtoken != NULL)
        fatal("Can't reject token twice");
    Rejtoken = t;
}


int scan(struct token *t) {
    int c, tokentype;

    // If we have any rejected token, return it
    if (Rejtoken != NULL) {
        // printf("Reject token is %d\n", Rejtoken); 
        t = Rejtoken;
        Rejtoken = NULL;
        return 1;
    }

    // skip
    c = skip();

    // Determine the token based on input char
    switch(c) {
        case EOF:
            t->token = T_EOF;
            return 0;
        case '+':
            if ((c = next()) == '+') {
                t->token = T_INC;
            } else {
                Putback = c;
                t->token = T_PLUS;
            }
            break;
        case '-':
            if ((c = next()) == '-') {
                t->token = T_DEC;
            } else {
                Putback = c;
                t->token = T_MINUS;
            }
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
        case '{':
            t->token = T_LBRACE;
            break;
        case '}':
            t->token = T_RBRACE;
            break;
        case '(':
            t->token = T_LPAREN;
            break;
        case ')':
            t->token = T_RPAREN;
            break;
        case '[':
            t->token = T_LBRACKET;
            break;
        case ']':
            t->token = T_RBRACKET;
            break;
        case '~':
            t->token = T_INVERT;
            break;
        case '^':
            t->token = T_XOR;
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
                Putback = c;
                t->token = T_LOGNOT;
            }
            break;
        case '<':
            if ((c = next()) == '=') {
                t->token = T_LE;
            } else if ((c = next()) == '<') {
                t->token = T_LSHIFT;
            } else {
                Putback = c;
                t->token = T_LT;
            }
            break;
        case '>':
            if ((c = next()) == '=') {
                t->token = T_GE;
            } else if ((c = next()) == '>') {
                t->token = T_RSHIFT;
            } else {
                Putback = c;
                t->token = T_GT;
            }
            break;
        case '&':
            if  ((c = next()) == '&') {
                t->token = T_LOGAND;
            } else {
                Putback = c;
                t->token = T_AMPER;
            }
            break;
        case '|':
            if ((c = next()) == '|') {
                t->token = T_LOGOR;
            } else {
                Putback = c;
                t->token = T_OR;
            }
            break;
        case '\'':
            // if it is a quote, scan in the 
            // literal character value and 
            // the trailing quote 
            t->intvalue = scanch();
            t->token = T_INTLIT;
            if (next() != '\'')
                fatal("Expected '\\'' at end of char literal");
            break;
        case '"':
            // scan in a literal string
            scanstr(Text);
            t->token = T_STRLIT;
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
                if ((tokentype = keyword(Text)) != 0) {
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

