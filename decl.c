#include "defs.h"
#include "scan.h"
#include "decl.h"

// Parse the current token and 
// return a primitive type enum value
int parse_type() {
    int type;
    switch (Token.token) {
        case T_VOID: type = P_VOID; break;
        case T_CHAR: type = P_CHAR; break;
        case T_INT:  type = P_INT;  break;
        case T_LONG: type = P_LONG; break;
        default:
        fatald("Illegal type, token", Token.token);
    }

    // scan in one or more further * tokens
    // and determine the correct pointer type
    while(1) {
        scan(&Token);
        if (Token.token != T_STAR) break;
        type = pointer_to(type);
    }

    // here we leave the next token already scanned
    return type;

}

// Parse the declaration of variable
// It must be the 'int' token followed by an identifier 
// and a semicolon. Add the identifier to symbol table
void var_declaration(int type) {
    int id;

    // After matchident() calling scan(),
    // Text now has the identifier's name
    // Add it as a known identifier
    // and generate its space in assembly
    id = addglob(Text,type, S_VARIABLE, 0);
    genglobsym(id);
    // Get the trailing semicolon
    matchsemi();
}

// Parse the declaration of a simplistic function
struct ASTnode *function_declaration(int type) {
    struct ASTnode *tree, *finalstmt;
    int nameslot, endlabel;

    
    // Get a label id for the end label (which can be jumped to)
    // Add the function to the symbol table
    // and set the Functional global to the function's symbol id
    endlabel = genlabel();
    nameslot = addglob(Text, type, S_FUNCTION, endlabel);
    Functionid = nameslot;

    matchlparen();
    matchrparen();

    // Get the AST for the body
    tree = compound_statement();

    // If the function type is not P_VOID, check that
    // the last AST operation in the compound statement was a return
    // statement
    if (type != P_VOID) {

        // Error if no statements in the function
        if (tree == NULL)
            fatal("No statements in function with non-void type");
        
        // Check that the last AST operation in the
        // compound statement was a return statement
        finalstmt = (tree->op == A_GLUE) ? tree->right : tree;
        if (finalstmt == NULL || finalstmt->op != A_RETURN)
            fatal("No return for function with non-void type");
    }

    // Return an A_FUNCTION node which has the function's nameslot
    // and the compound statement sub-tree
    return mkastunary(A_FUNCTION, P_VOID, tree, nameslot);
}

// Parse one or more global declarations 
// either variables or functions
void global_declarations(void) {
    struct ASTnode *tree;
    int type;

    while (1) {
        // Firstly parse the type and identifier
        // to see either a '(' for a function declaration
        // or a ',' or ';' for a variable declaration.
        type = parse_type();
        matchident();
        if (Token.token == T_LPAREN) {
            tree = function_declaration(type);
            genAST(tree, NOREG, 0);
        } else {
            // Parse the global variable declaration
            var_declaration(type);
        }

        // Stop when we have reached EOF
        if (Token.token == T_EOF)
            break;
    }
}