#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse a single statement and return its AST
struct ASTnode *single_statement(void) {
    switch(Token.token) {
        case T_PRINT:
            return print_statement();
        case T_CHAR:
        case T_INT:
            var_declaration();
            return NULL;
        case T_IDENT:
            return assignment_statement();
        case T_IF:
            return if_statement();
        case T_WHILE:
            return while_statement();
        case T_FOR:
            return for_statement();
        case T_RETURN:
            return return_statement();
        default:
            fatald("Syntax error, token", Token.token);
    }
}

// The Part 8 BNF grammar
// compound_statement:
//      |   statement
//      |   statement statements
//      ;
//
//
// statement: print_statement
//      |   declaration
//      |   assignment_statement
//      |   if_statement
//      ;
// 
// print_statement: 'print' expression ';' ;
//
// declaration: 'int' identifier ';' ;
// 
// assignment_statement:  identifier '=' expression ';' ;
// 
// if_statement: if_head
//      |        if_head 'else' compound_statement
//      ;
// if_head: 'if' '(' true_false_expression ')' compound_statement ;
//
// identifier: T_IDENT ;

// Parse a compound statement and return its AST
// We will glue multiple (unrelated) trees into one single tree
// We glue the tree from left to right, bottom to up
// for example. if there are statements 
// stmt1;
// stmt2;
// stmt3;
// stmt4; 
// they will be glued into a tree
    //          A_GLUE
    //           /  \
    //       A_GLUE stmt4
    //         /  \
    //     A_GLUE stmt3
    //       /  \
    //   stmt1  stmt2
struct ASTnode *compound_statement(void) {
    struct ASTnode *left = NULL;
    struct ASTnode *tree;

    // Require a left curly bracket
    matchlbrace();

    while (1) {

        // Parse a single statement
        tree = single_statement();

        // if the single statement is print statement or assignment statement, 
        // need to match the semicolon at last.
        if (tree != NULL && (tree->op == A_ASSIGN || tree->op == A_PRINT ||
                             tree->op == A_FUNCCALL || tree->op == A_RETURN)) {
            matchsemi();
        }

        // For each new tree, either save it as left if left is empty
        // or glue left and tree together
        if (tree != NULL) {
            if (left == NULL)
                left = tree;
            else
                left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, 0);
        }

        // When we hit a right curly bracket, 
        // skip it and return AST
        if (Token.token == T_RBRACE) {
            matchrbrace();
            return left;
        }
    }
}

struct ASTnode *if_statement(void) {
    struct ASTnode *condAST, *trueAST, *falseAST = NULL;

    // First match "if" keyword and lparen
    match(T_IF, "if");
    matchlparen();

    // Parse the condition expression (so far the 6 comparator version in Part8)
    // then parse the rparen
    condAST = binexpr(0);
    
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
    matchrparen();

    // Get the AST for the compound statement
    trueAST = compound_statement();

    // If we have an 'else', skip it and
    // parse its compound statement
    if (Token.token == T_ELSE) {
        scan(&Token);
        falseAST = compound_statement();
    }
    
    // build and return the AST for if statement
    return mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, 0);

}

struct ASTnode *print_statement(void) {
    struct ASTnode *tree;
    int lefttype, righttype;
    int reg;

    // Match a 'print' as the first token
    match(T_PRINT, "print");

    // Parse the following expression and
    // generate the assembly code
    tree = binexpr(0);
    // printf("%d\n", tree->op);
    
    // Ensure two types are compatible
    lefttype = P_INT;
    righttype = tree->type;
    if (!type_compatible(&lefttype, &righttype, 0))
        fatal("Incompatible types");

    // Widen the tree if required, when printing a char
    if (righttype)
        tree = mkastunary(righttype, P_INT, tree, 0);
    // make a print AST tree
    tree = mkastunary(A_PRINT, P_NONE, tree, 0);

    return tree;
}

// In assignment, identifier is the rvalue. the operation for the rvalue node is A_LVIDENT
struct ASTnode *assignment_statement(void) {
    struct ASTnode *left, *right, *tree;
    int lefttype, righttype;
    int id;

    // Ensure we have an identifier
    matchident();

    // This could be a variable or a function call
    // If next token is '(', it's a function call
    if (Token.token == T_LPAREN) 
        return funccall();

    // Not a function call
    // check if it has been defined and make leaf node for it
    if ((id = findglob(Text)) == -1) {
        fatals("Undeclared variable", Text);
    }
    // printf("%s\n", Gsym[id]->name);
    right = mkastleaf(A_LVIDENT, Gsym[id]->type, id);

    match(T_ASSIGN, "=");

    // Rvalue expression needs to be evaluated before saved to variable, make it left tree.
    left = binexpr(0);

    // Ensure the two types are compatible
    lefttype = left->type;
    righttype = right->type;
    if (!type_compatible(&lefttype, &righttype, 1)) // a wide value (expression) cannot be assigned to a narrow variable
        fatal("Incompatible types");
    
    // Widen the left if required
    if (lefttype)
        left = mkastunary(lefttype, right->type, left, 0);
    
    // Make an assignment AST tree
    tree = mkastnode(A_ASSIGN, P_INT, left, NULL, right, 0);

    return tree;
}

struct ASTnode *while_statement(void) {
    struct ASTnode *condAST, *bodyAST;

    // Firstly we match "while" keyword and left paren
    match(T_WHILE, "while");
    matchlparen();

    // Parse the condition expression
    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
    matchrparen();

    // Parse the body compound statement
    bodyAST = compound_statement();


    return mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, 0);

}

// Parse a FOR statement and return its AST
struct ASTnode* for_statement(void) {
    struct ASTnode *condAST, *bodyAST;
    struct ASTnode *preopAST, *postopAST;
    struct ASTnode *tree;

    // Firstly match keyword 'for' and '('
    match(T_FOR, "for");
    matchlparen();

    // get the pre_op statement and ;
    preopAST = single_statement();
    matchsemi();

    // get the condition statement and ;
    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
    matchsemi();

    // get the post_op statement and the ')'
    postopAST = single_statement();
    matchrparen();

    // get the body compound statement
    bodyAST = compound_statement();

    // A FOR statement (pre_op; condition; post_op) {body}
    // can be transformed into a WHILE statement
    // pre_op
    // while (condition) {
    //    body
    //    post_op
    // }
    // So we can just use while statemeht, and glue pre_op, condition, body, post_op together
    // such as
    //              
    //              A_GLUE
    //              /     \
    //          preop     A_WHILE
    //                     /    \
    //               decision  A_GLUE
    //                         /    \
    //                     compound  postop
    tree = mkastnode(A_GLUE, P_NONE, bodyAST, NULL, postopAST, 0);

    tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, 0);

    tree = mkastnode(A_GLUE, P_NONE, preopAST, NULL, tree, 0);

    return tree;
}

// Parse a return statment and return its AST
static struct ASTnode *return_statement(void) {
    struct ASTnode *tree;
    int returntype, functype;

    // Can't return a value if function returns P_VOID
    if (Gsym[Functionid]->type == P_VOID)
        fatal("Can't return from a void function");
    
    // Ensure we have 'return' '('
    match(T_RETURN, "return");
    matchlparen();
    // Parse the following expression
    tree = binexpr(0);

    // Check the return type is compatible with function's type
    returntype = tree->type;
    functype = Gsym[Functionid]->type;
    // Here, return type can be wider than functype, so 1 here to prevent
    // they are compatible (see type_compatible() in types.c)
    if (!type_compatible(&returntype, &functype, 1))
        fatal("Incompatible types");
    
    // Widen left if necessary
    if (returntype)
        tree = mkastunary(returntype, functype, tree, 0);

    // Add on the A_RETURN node
    tree = mkastunary(A_RETURN, P_NONE, tree, 0);

    // Get the ')'
    matchrparen();

    return tree;
}