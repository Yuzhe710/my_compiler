#include "defs.h"
#include "scan.h"
#include "decl.h"

// Given a primitive type, return
// the type which is a pointer to it
int pointer_to(int type) {
    if ((type & 0xf) == 0xf)
        fatald("Unrecognised in pointer_to: type", type);
    return type + 1;
}

// Given a primitive type, return 
// the type which it points to
// As long as we have an integer type, the lowest four bits are 0s.
// hence if an integer type & 0xf and we will get 0x0, meaning it cannot
// be dereferenced
int value_at(int type) {
    if ((type & 0xf) == 0x0)
        fatald("Unrecognised in value_at: type", type);
    return type - 1;
}

// Return true if a type is an int type of any size
// false otherwise
int inttype(int type) {
    return ((type & 0xf) == 0);
}

// Return true if a type is of pointer type
int ptrtype(int type) {
    return ((type & 0x1f) != 0);
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
        if (rsize > lsize) return mkastunary(A_WIDEN, rtype, tree, NULL,  0);
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
                return mkastunary(A_SCALE, rtype, tree, NULL, rsize);
            else
                return tree;
        }
    }

    // When we get here, types are not compatible
    return NULL;
}