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
// class is the variable's class
struct symtable *var_declaration(int type, int class) {

    struct symtable *sym = NULL;

    // see if this has already been declared
    switch(class) {
        case C_GLOBAL:
            if (findglob(Text) != NULL)
                fatals("Duplicate global variable declaration", Text);
        case C_LOCAL:
        case C_PARAM:
            if (findlocl(Text) != NULL)
                fatals("Duplicate local variable declaration", Text);
    }

    // Text now has the identifier's name
    // If the next token is a '[' 
    if (Token.token == T_LBRACKET) {
        // skip past '['
        scan(&Token);
        // Check we have an array size
        if (Token.token == T_INTLIT) {
            // Add this as a known array and generate its space in assembly
            // We treat the array as a pointer to its elements' type
            switch(class) {
                case C_GLOBAL:
                    sym = addglob(Text, pointer_to(type), S_ARRAY, class, Token.intvalue);
                    break;
                case C_LOCAL:
                case C_PARAM:
                    fatal("For now, declaration of local arrays is not implemented");
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
        switch (class) {
            case C_GLOBAL:
                sym = addglob(Text, type, S_VARIABLE, class, 1);
                break;
            case C_LOCAL:
                sym = addlocl(Text, type, S_VARIABLE, class, 1);
                break;
            case C_PARAM:
                sym = addparm(Text, type, S_VARIABLE, class, 1);
                break;
        }
    }
    return sym;
}

// param_declaration: <null>
//           | variable_declaration
//           | variable_declaration ',' param_declaration
// 
// Parse the parameters in parenthesis after the function name.
// Add them as symbols to the symvol table and return the number
// of parameters. If id is not -1, there is an existing function 
// prototype, and the function has this symbol slot number
static int param_declaration(struct symtable *funcsym) {
    int type;
    int numparam = 0;
    struct symtable *protoptr = NULL;

    // If there is a prototype, get the pointer 
    // to the first prototype parameter
    if (funcsym != NULL)
        protoptr = funcsym->member;
    

    // Loop until the final right parenthesis
    while (Token.token != T_RPAREN) {
        // Get the type and identifier
        // add it to the symbol table
        type = parse_type();
        matchident();

        // If we have an existing prototype
        // Check that this type matches the prototype
        if (protoptr != NULL) {
            if (type != protoptr->type)
                fatald("Type doesn't match prototype for parameter", numparam + 1);
            protoptr = protoptr->next;
        } else {
            // Add a new parameter to the new prototype
            var_declaration(type, C_PARAM);
        }
        numparam++;

        // Must have a ',' or ')' at this point
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

    // Check that the number of parameters in this list matches 
    // any existing prototype
    if ((funcsym != NULL) && (numparam != funcsym->nelems)) 
        fatals("Parameter count mismatch for function", funcsym->name);

    // Return the count of parameters
    return numparam;
}

// function_delcaration: type identifier '(' parameter_list ')'
//                          | type identifier '(' parameter_list ')' compound_statement ;
//
// Parse the declaration of a function
// The identifier has been scanned & we have the type
struct ASTnode *function_declaration(int type) {
    struct ASTnode *tree, *finalstmt;
    struct symtable *oldfuncsym, *newfuncsym = NULL;
    int endlabel, numparam;

    // Text has the identifier's name. If this exists and is a 
    // function, get the id. Otherwise, set oldfuncsym to NULL.
    if ((oldfuncsym = findsymbol(Text)) != NULL)
        if (oldfuncsym->stype != S_FUNCTION)
            oldfuncsym = NULL;
    
    // If this is a new function declaration, get a 
    // label-id for the end label, and add the function
    // to the symbol table
    if (oldfuncsym == NULL) {
        endlabel = genlabel();
        newfuncsym = addglob(Text, type, S_FUNCTION, C_GLOBAL, endlabel);
    }

    // Scan in the '(', any parameters and the ')'.
    // Pass in any existing function prototype symbol slot number
    matchlparen();
    numparam = param_declaration(oldfuncsym);
    matchrparen();

    // If this is a new function declaration, update the 
    // function symbol entry with the number of parameters
    // Also copy the parameter list into the function's node
    if (newfuncsym) {
        newfuncsym->nelems = numparam;
        newfuncsym->member = Parmhead;
        oldfuncsym = newfuncsym;
    }

    // Clear the parameter list
    Parmhead = Parmtail = NULL;

    // Declaration ends in a semicolon, only a prototype.
    if (Token.token == T_SEMI) {
        scan(&Token);
        return NULL;
    }

    // This is not just a prototype
    // Set the Functionid global to the function's symbol pointer
    Functionid = oldfuncsym;

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
    return mkastunary(A_FUNCTION, type, tree, oldfuncsym, endlabel);
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
            // only a function prototype, no code
            if (tree == NULL)
                continue;
            // A real function, generate the assembly code for it
            genAST(tree, NOLABEL, 0);
            // Now free the symbols associated with this function
            freeloclsyms();
        } else {
            // Parse the global variable declaration, here the scope is global
            var_declaration(type, C_GLOBAL);
            matchsemi();
        }

        // Stop when we have reached EOF
        if (Token.token == T_EOF)
            break;
    }
}