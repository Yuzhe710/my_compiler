#include "defs.h"
#include "scan.h"
#include "decl.h"

// Given two primitive types,
// return true if they are compatible,
// false otherwise. Also return either 
// zero or an A_WIDEN operation if one 
// has to be widened to match the other
// If onlyright is true, only widen left to right
int type_compatible(int *left, int *right, int onlyright) {
    int leftsize, rightsize;

    // Same type, they are compatible
    if (*left == *right) {
        *left = *right = 0;
        return 1;
    }

    // Get the sizes for each type
    leftsize = genprimsize(*left);
    rightsize = genprimsize(*right);

    // Types with zero size are not compatible with anything
    if ((leftsize == 0) || (rightsize == 0))
        return 0;
    
    // Widen the left types as required, lefttypes are from the 
    // expression to be assigned to variable
    // function's return type 
    // P_INT in print statement
    if (leftsize < rightsize) {
        *left = A_WIDEN;
        *right = 0;
        return 1;
    }

    if (rightsize < leftsize) {
        // onlyright is used when check compatibility between 
        // left = return type, right = func type
        // left = expression type, right = variable type
        // hence the narrower right type should not hold the left value
        if (onlyright) return 0;
        *left = 0;
        *right = A_WIDEN;
        return 1;
    }

    // Anything remaining is the same size
    // thus compatible
    *left = *right = 0;
    return 1;
}

// Given a primitive type, return
// the type which is a pointer to it
int pointer_to(int type) {
    int newtype;
    switch (type) {
        case P_VOID: newtype = P_VOIDPTR; break;
        case P_CHAR: newtype = P_CHARPTR; break;
        case P_INT: newtype = P_INTPTR; break;
        case P_LONG: newtype = P_LONGPTR; break;
        default:
            fatald("Unrecognised in pointer_to: type", type);
    }
    return newtype;
}

// Given a primitive type, return 
// the type which it points to
int value_at(int type) {
    int newtype;
    switch (type) {
        case P_VOIDPTR: newtype = P_VOID; break;
        case P_CHARPTR: newtype = P_CHAR; break;
        case P_INTPTR:  newtype = P_INT;  break;
        case P_LONGPTR: newtype = P_LONG; break;
        default:
        fatald("Unrecognised in value_at: type", type);
    }
    return newtype;
}

// Return true if a type is an int type of any size
// false otherwise
int inttype(int type) {
    if (type == P_CHAR || type == P_INT || type == P_LONG)
        return 1;
    return 0;
}

// Return true if a type is of pointer type
int ptrtype(int type) {
    if (type == P_VOIDPTR ||  type == P_CHARPTR || type == P_INTPTR || type == P_LONGPTR) 
        return 1;
    return 0;
}   

// Given an AST tree and a type which we want it to become, 

struct ASTnode *modify_type(struct ASTnode *tree, int rtype, int op) {
    int ltype;
    int lsize, rsize;

    ltype = tree->type;

    // Compare scalar int types
    if (inttype(ltype) && inttype(rtype)) {

        // Both types are the same, nothing to do
        if (ltype == rtype) return tree;

        // Get the size for each side
        lsize = genprimsize(ltype);
        rsize = genprimsize(rtype);

        // Tree's size too big
        if (lsize > rsize) return NULL;

        // Widen to the right
        if (rsize > lsize) return mkastunary(A_WIDEN, rtype, tree, 0);
    }

    // For pointers on the left
    // I actually have not get too much about the usage here
    // maybe only the case when doing pointer_1 = pointer_2
    if (ptrtype(ltype)) {
        // if left and right are the same type and no any binary op
        // it is OK
        if (op == 0 && ltype == rtype) return tree;
    }

    // Scale. only for A_ADD or A_SUBSTRACT operation
    // Used when have like &a + 1, or someone may write 1 + &a
    // will be checked two way around
    if (op == A_ADD || op == A_SUBSTRACT) {
        
        // Left is int type, right is pointer type
        // size of the original type > 1, we scale the left
        if (inttype(ltype) && ptrtype(rtype)) {
            rsize = genprimsize(value_at(rtype));
            if (rsize > 1)
                return mkastunary(A_SCALE, rtype, tree, rsize);
            else
                return tree;
        }
    }

    // When we get here, types are not compatible
    return NULL;
}