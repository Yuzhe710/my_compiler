
// scan.c
int scan(struct token *t);
void reject_token(struct token* t);

// tree.c
struct ASTnode *mkastnode(int op, int type, struct ASTnode *left, struct ASTnode *mid, struct ASTnode *right, int intvalue);
struct ASTnode *mkastleaf(int op, int type, int intvalue);
struct ASTnode *mkastunary(int op, int type, struct ASTnode *left, int intvalue);

// expr.c
struct ASTnode *binexpr(int rbp);
struct ASTnode *funccall(void);

// gen.c
int genAST(struct ASTnode *n, int reg, int parentASTop);
void genpreamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(int id);
int genlabel(void);
int genprimsize(int type);


//int interpretAST(struct ASTnode *n);
// void generatecode(struct ASTnode *n);

//cg.c
void freeall_registers(void);
void cgpreamble();
void cgfuncpreamble(int id);
void cgfuncpostamble(int id);
int cgloadint(int value, int type);
int cgloadglob(int id);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
int cgstorglob(int r, int id);
void cgprintint(int r);
void cgglobsym(int id);
int cgcompare_and_set(int ASTop, int r1, int r2);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);
void cglabel(int l);
void cgjump(int l);
int cgwiden(int r, int oldtype, int newtype);
int cgprimsize(int type);
int cgcall(int r, int id);
void cgreturn(int reg, int id);

// stmt.c
struct ASTnode *single_statement(void);
struct ASTnode *compound_statement(void);
struct ASTnode *print_statement(void);
struct ASTnode *if_statement(void);
struct ASTnode *assignment_statement(void);
struct ASTnode *while_statement(void);
struct ASTnode *for_statement(void);
static struct ASTnode *return_statement(void);

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
int addglob(char* name, int type, int stype, int endlabel);

// decl.c
int parse_type(int t);
void var_declaration(void);

struct ASTnode *function_declaration(void);

// types.c
int type_compatible(int *left, int *right, int onlyright);