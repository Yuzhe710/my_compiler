
// scan.c
int scan(struct token *t);
void reject_token(struct token* t);

// tree.c
struct ASTnode *mkastnode(int op, int type, struct ASTnode *left, struct ASTnode *mid, struct ASTnode *right, struct symtable *sym, int intvalue);
struct ASTnode *mkastleaf(int op, int type, struct symtable *sym, int intvalue);
struct ASTnode *mkastunary(int op, int type, struct ASTnode *left, struct symtable *sym, int intvalue);
void dumpAST(struct ASTnode *n, int label, int level);

// expr.c
struct ASTnode *binexpr(int rbp);
struct ASTnode *funccall(void);
struct ASTnode *prefix(void);
struct ASTnode *postfix(void);

// gen.c
int genAST(struct ASTnode *n, int reg, int parentASTop);
void genpreamble();
void genfreeregs();
void genprintint(int reg);
void genglobsym(struct symtable *node);
int genlabel(void);
int genprimsize(int type);
int genglobstr(char *strvalue);
void genpostamble();


//int interpretAST(struct ASTnode *n);
// void generatecode(struct ASTnode *n);

//cg.c
void freeall_registers(void);
void cgpreamble();
void cgfuncpreamble(struct symtable *sym);
void cgfuncpostamble(struct symtable *sym);
int cgloadint(int value, int type);
int cgadd(int r1, int r2);
int cgsub(int r1, int r2);
int cgmul(int r1, int r2);
int cgdiv(int r1, int r2);
int cgstorglob(int r, struct symtable *sym);
void cgprintint(int r);
void cgglobsym(struct symtable *node);
int cgcompare_and_set(int ASTop, int r1, int r2);
int cgcompare_and_jump(int ASTop, int r1, int r2, int label);
void cglabel(int l);
void cgjump(int l);
int cgwiden(int r, int oldtype, int newtype);
int cgprimsize(int type);
int cgcall(struct symtable *sym, int numargs);
void cgreturn(int reg, struct symtable *sym);
int cgaddress(struct symtable *sym);
int cgderef(int r, int type);
int cgshlconst(int r, int val);
int cgstorderef(int r1, int r2, int type);
void cgglobstr(int l, char *strvalue);
int cgloadglobstr(int id);
int cgand(int r1, int r2);
int cgor(int r1, int r2);
int cgxor(int r1, int r2);
int cgnegate(int r);
int cginvert(int r);
int cgshl(int r1, int r2);
int cgshr(int r1, int r2);
int cglognot(int r);
int cgboolean(int r, int op, int label);
int cgloadglob(struct symtable *sym, int op);
void cgtextseg(void);
void cgdataseg(void);
void cgresetlocals(void);
int cgloadlocal(struct symtable *sym, int op);
int cgloadglobstr(int id);
int cgstorlocal(int r, struct symtable *sym);
void cgcopyarg(int r, int argposn);
void cgpostamble(void);



// stmt.c
struct ASTnode *single_statement(void);
struct ASTnode *compound_statement(void);
struct ASTnode *if_statement(void);
struct ASTnode *while_statement(void);
struct ASTnode *for_statement(void);
struct ASTnode *return_statement(void);

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
struct symtable *findglob(char *s);
struct symtable *findlocl(char *s);
struct symtable *findcomposite(char *s);
struct symtable *addglob(char* name, int type, int stype, int class, int size);
struct symtable *addlocl(char *name, int type, int stype, int class, int size);
struct symtable *findsymbol(char *s);
struct symtable *addparm(char *name, int type, int stype, int class, int size);
struct symtable *newsym(char *name, int type, int stype, int class,
			int size, int posn);
void clear_symtable(void);
void freeloclsyms(void);
void appendsym(struct symtable **head, struct symtable **tail, struct symtable *node);

// decl.c
int parse_type(void);
struct symtable *var_declaration(int type, int class);
struct ASTnode *function_declaration(int type);
void global_declarations(void);

// types.c
int pointer_to(int type);
int value_at(int type);
int inttype(int type);
int ptrtype(int type);
struct ASTnode *modify_type(struct ASTnode *tree, int rtype, int op);