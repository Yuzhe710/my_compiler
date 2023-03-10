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

// variable_delaration: type identifier ';'
//      | type identifier '[' INTLIT ']' ':'
//      ;
// 
// Parse the declaration of a scalar variable or an array
// with a given size.
// The identifier has already been scanned and we already have the type
// islocal is set if this is a local variable
// isparam is set if this local variable is a function parameter
void var_declaration(int type, int islocal, int isparam) {

    // Text now has the identifier's name
    // If the next token is a '[' 
    if (Token.token == T_LBRACKET) {
        // skip past '['
        scan(&Token);
        // Check we have an array size
        if (Token.token == T_INTLIT) {
            // Add this as a known array and generate its space in assembly
            // We treat the array as a pointer to its elements' type
            if (islocal) {
                // addlocl(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
                fatal("For now, declaration of local arrays is not implemented");
            } else {
                addglob(Text, pointer_to(type), S_ARRAY, 0, Token.intvalue);
            }
        }

        // Ensure we have a following ']'
        scan(&Token);
        match(T_RBRACKET, "]");
    } else {
        // After matchident() calling scan(),
        // Text now has the identifier's name
        // Add it as a known identifier
        // and generate its space in assembly
        if (islocal) {
            if (addlocl(Text, type, S_VARIABLE, isparam, 1) == -1)
                fatals("Duplicate local variable declaration", Text);
        } else {
            addglob(Text, type, S_VARIABLE, 0, 1);
        }
    }
    
    // Get the trailing semicolon
    // matchsemi();
}

// param_declaration: <null>
//           | variable_declaration
//           | variable_declaration ',' param_declaration
// 
// Parse the parameters in parenthesis after the function name.
// Add them as symbols to the symvol table and return the number
// of parameters
static int param_declaration(void) {
    int type;
    int numparam = 0;

    // Loop until the final right parenthesis
    while (Token.token != T_RPAREN) {
        // Get the type and identifier
        // add it to the symbol table
        type = parse_type();
        matchident();
        var_declaration(type, 1, 1);
        numparam++;
        // Must have a ',' or ')' at the point
        switch (Token.token) {
            case T_COMMA: 
                scan(&Token);
                break;
            case T_RPAREN:
                break;
            default:
                fatald("Unexpected token in parameter list", Token.token);
        }
    }

    // Return the count of parameters
    return numparam;
}

// Parse the declaration of a simplistic function
struct ASTnode *function_declaration(int type) {
    struct ASTnode *tree, *finalstmt;
    int nameslot, endlabel, numparam;

    
    // Get a label id for the end label (which can be jumped to)
    // Add the function to the symbol table
    // and set the Functional global to the function's symbol id
    endlabel = genlabel();
    nameslot = addglob(Text, type, S_FUNCTION, endlabel, 0);
    Functionid = nameslot;

    // genresetlocals();   // Reset position of new locals

    matchlparen();
    numparam = param_declaration();
    printf("%s\n", Symtable[1016]->name);
    Symtable[nameslot]->nelems = numparam;
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
    return mkastunary(A_FUNCTION, type, tree, nameslot);
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

            // Now free the symbols associated with this function
            freelocsyms();
        } else {
            // Parse the global variable declaration, here the scope is global
            var_declaration(type, 0, 0);
            matchsemi();
        }

        // Stop when we have reached EOF
        if (Token.token == T_EOF)
            break;
    }
}