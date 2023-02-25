#include "defs.h"
#include "scan.h"
#include "decl.h"

static int Globs = 0;       // Position of next free global symbol slot

// Determine if the symbol is in the global symbol table
// Return its slot position or -1 if not found
int findglob(char *s) {
    int i;

    for(i = 0; i < Globs; i ++) {
        if ((*s == *(Gsym[i]->name)) && !strcmp(s, Gsym[i]->name))
            return i;
    }
    return -1;
}

// Get a position of a new global symbol slot, or die
// if we run out of positions
static int newglob(void) {
    int p;

    if ((p = Globs++) >= NSYMBOLS)
        fatal("Too many global symbols");
    return p;
}

// Add a global symbol to the symbol table
// Return the slot number in the symbol table
int addglob(char *name, int type, int stype, int endlabel, int size) {
    int y;

    // if the symbol is already in symbol table
    // return the existing slot
    if ((y = findglob(name)) != -1)
        return y;
    
    // otherwise create a new slot and return its number
    y = newglob();
    Gsym[y]->name = strdup(name);
    Gsym[y]->type = type;
    Gsym[y]->stype = stype;
    Gsym[y]->endlabel = endlabel;
    Gsym[y]->size = size;
    return y;
}