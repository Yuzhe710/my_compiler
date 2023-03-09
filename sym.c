#include "defs.h"
#include "scan.h"
#include "decl.h"


// Determine if the symbol is in the global symbol table
// Return its slot position or -1 if not found
// Skip C_PARAM entries
int findglob(char *s) {
    int i;

    for(i = 0; i < Globs; i ++) {
        if (Symtable[i]->class == C_PARAM) continue;
        if ((*s == *(Symtable[i]->name)) && !strcmp(s, Symtable[i]->name))
            return i;
    }
    return -1;
}

// Determine if the symbol s is in the local symbol table
// Return its slot position or -1 if not found
int findlocl(char *s) {
    int i;

    for (i = Locls + 1; i < NSYMBOLS; i++) {
        if (*s == *Symtable[i]->name && !strcmp(s, Symtable[i]->name))
            return i;
    }
    return -1;
}

// Get the position of a new global symbol slot, or die 
// if we've run out of positions.
static int newglob(void) {
    int p;

    if ((p = Globs++) >= Locls)
        fatal("Too many global symbols");
    return p;
}

// Get the positon of a new local symbol slot, or die 
// if we've run out of positions.
static int newlocl(void) {
    int p;

    if ((p = Locls--) <= Globs)
        fatal("Too many local symbols");
    return p;
}

// Update a symbol at the given slot number in the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// + endlabel: if this is a function
// + posn: Position information for local symbols 
static void updatesym(int slot, char *name, int type, int stype, 
        int class, int endlabel, int size, int posn) {
    if (slot < 0 || slot >= NSYMBOLS)
        fatal("Invalid symbol slot number in updatesym()");
    Symtable[slot]->name = strdup(name);
    Symtable[slot]->type = type;
    Symtable[slot]->stype = stype;
    Symtable[slot]->class = class;
    Symtable[slot]->endlabel = endlabel;
    Symtable[slot]->size = size;
    Symtable[slot]->posn = posn;
}

// Clear all the entries in the
// local symbol table
void freelocsyms(void) {
  Locls = NSYMBOLS - 1;
}

// Add a global symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc
// + size: number of elements
// + endlabel: if this is a function
// Return the slot number in the symbol table
int addglob(char *name, int type, int stype, int endlabel, int size) {
    int slot;

    // if the symbol is already in symbol table
    // return the existing slot
    if ((slot = findglob(name)) != -1)
        return slot;
    
    // otherwise create a new slot and return its number
    slot = newglob();
    updatesym(slot, name, type, stype, C_GLOBAL, endlabel, size, 0);
    genglobsym(slot);
    return slot;
}

// Add a local symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// + isparam: if true, this is a parameter to the function
// Return the slot number in the symbol table, -1 if a duplicate entry
int addlocl(char *name, int type, int stype, int isparam, int size) {
    int localslot, globalslot;
    
    // If this is already in the symbol table, return the existing slot
    if ((localslot = findlocl(name)) != -1)
        return -1;
    
    // otherwise get a new symbol slot and a position for this local.
    // Update the symbol table entry. If this is a parameter, 
    // also create a global C_PARAM entry to build the function's prototype
    localslot = newlocl();
    if (isparam) {
        updatesym(localslot, name, type, stype, C_PARAM, 0, size, 0);
        globalslot = newglob();
        updatesym(globalslot, name, type, stype, C_PARAM, 0, size, 0);
    } else {
        updatesym(localslot, name, type, stype, C_LOCAL, 0, size, 0);
    }
    
    return localslot;
}

// Determine if the symbol s is in the symbol table.
// Return its slot position or -1 if not found
// Will firstly try to find if it is a local symbol
// then try to find if it is global symbol
int findsymbol(char *s) {
    int slot;

    slot = findlocl(s);
    if (slot == -1)
        slot = findglob(s);
    return slot;
}