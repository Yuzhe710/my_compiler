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