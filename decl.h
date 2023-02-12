
// scan.c
int scan(struct token *t);

// tree.c
struct ASTnode *mkastnode(int op, struct ASTnode *left, struct ASTnode *mid, struct ASTnode *right, int intvalue);
struct ASTnode *mkastleaf(int op, int intvalue);
struct ASTnode *mkastunary(int op, struct ASTnode *left, int intvalue);

// expr.c
struct ASTnode *binexpr(int rbp);

// gen.c
int genAST(struct ASTnode *n, int reg, int parentASTop);
void genpreamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(char *s);

//int interpretAST(struct ASTnode *n);
// void generatecode(struct ASTnode *n);

//cg.c
void freeall_registers(void);
void cgpreamble();
void cgfuncpreamble(char *name);
void cgfuncpostamble();
int cgloadint(int value);
int cgloadglob(char *identifier);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
int cgstorglob(int r, char *identifier);
void cgprintint(int r);
void cgglobsym(char *sym);
int cgcompare_and_set(int ASTop, int r1, int r2);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);
void cglabel(int l);
void cgjump(int l);


// stmt.c
struct ASTnode *single_statement(void);
struct ASTnode *compound_statement(void);
struct ASTnode *print_statement(void);
struct ASTnode *if_statement(void);
struct ASTnode *assignment_statement(void);
struct ASTnode *while_statement(void);
struct ASTnode *for_statement(void);

// misc.c
void match(int t, char *what);
void matchsemi(void);
void matchident(void);
void matchlbrace(void);
void matchrbrace(void);
void matchlparen(void);
void matchrparen(void);
void fatal(char *s);
void fatals(char *s1, char *s2);
void fatald(char *s, int d);
void fatalc(char *s, int c);

// sym.c
int findglob(char *s);
int addglob(char* name);

// decl.c
void var_declaration(void);
struct ASTnode *function_declaration(void);