#include "scan.h"
#include "decl.h"
#include "defs.h"

// Check that if current token is t
// fetch next token if yes, else throw an error
void match(int t, char *what) {
    if (Token.token == t) {
        scan(&Token);
    } else {
        printf("%s expected on line %d\n", what, Line);
        exit(1);
    }
}

// check if current token is semicolon
void matchsemi(void) {
    match(T_SEMI, ";");
}

void matchident(void) {
    match(T_IDENT, "identifier");
}

void matchlbrace(void) {
    match(T_LBRACE, "{");
}

void matchrbrace(void) {
    match(T_RBRACE, "}");
}

void matchlparen(void) {
    match(T_LPAREN, "(");
}

void matchrparen(void) {
    match(T_RPAREN, ")");
}



// Print out fatal messages
void fatal(char *s) {
  fprintf(stderr, "%s on line %d\n", s, Line); 
  fclose(Outfile);
  unlink(Outfilename);  // remove files if they never fully generated 
  exit(1);
}

void fatals(char *s1, char *s2) {
  fprintf(stderr, "%s:%s on line %d\n", s1, s2, Line); exit(1);
}

void fatald(char *s, int d) {
  fprintf(stderr, "%s:%d on line %d\n", s, d, Line); exit(1);
}

void fatalc(char *s, int c) {
  fprintf(stderr, "%s:%c on line %d\n", s, c, Line); exit(1);
}
