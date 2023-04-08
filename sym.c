#include "defs.h"
#include "scan.h"
#include "decl.h"


// Append a node to the singly-linked list 
void appendsym(struct symtable **head, struct symtable **tail,
        struct symtable *node) {

    // Check for valid pointers 
    if (head == NULL || tail == NULL || node == NULL)
        fatal("Either head, tail or node is NULL in appendsym");

    // Append to the list
    if (*tail) {
        (*tail)->next = node;
        *tail = node;
    } else {
        *head = *tail = node;
    }
    node->next == NULL;

}

// Create a symbol node to be added to a symbol table list
// Set up the nodes
// + type: char, int etc
// + structural type: var, function, array etc.
// + size: number of elements, or endlabel: end label for a function
// + posn: Position information for local symbols
// Return a pointer to the new node
struct symtable *newsym(char *name, int type, int stype, int class, 
        int size, int posn) {
    
    // Get a new node
    struct symtable *node = (struct symtable *) malloc(sizeof(struct symtable));
    if (node == NULL)
        fatal("Unable to malloc a symbol table node in newsym");
    
    // Fill in the values
    node->name = strdup(name);
    node->type = type;
    node->stype = stype;
    node->class = class;
    node->size = size;
    node->posn = posn;
    node->next = NULL;
    node->member = NULL;

    // Generate any gloabl space
    if (class == C_GLOBAL)
        genglobsym(node);
    return node;
}

// Search for a symbol in a specific list.
// Return a pointer to the found node or NULL if not found
static struct symtable *findsyminlist(char *s, struct symtable *list) {
    for (; list != NULL; list = list->next)
        if ((list->name != NULL) && !strcmp(s, list->name))
            return list;
    return NULL;
}

// Determine if the symbol is in the global symbol table
// Return its slot position or -1 if not found
// Skip C_PARAM entries
struct symtable *findglob(char *s) {
    return (findsyminlist(s, Globhead));
}

// Determine if the symbol s is in the local symbol table
// Return its slot position or -1 if not found
struct symtable *findlocl(char *s) {
    struct symtable *node;

    // Look for a parameter if we are in a function's body
    if (Functionid) {
        node = findsyminlist(s, Functionid->member);
        if (node)
            return node;
    }
    return findsyminlist(s, Loclhead);
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
// static void updatesym(int slot, char *name, int type, int stype, 
//         int class, int size, int posn) {
//     if (slot < 0 || slot >= NSYMBOLS)
//         fatal("Invalid symbol slot number in updatesym()");
//     Symtable[slot]->name = strdup(name);
//     Symtable[slot]->type = type;
//     Symtable[slot]->stype = stype;
//     Symtable[slot]->class = class;
//     Symtable[slot]->size = size;
//     Symtable[slot]->posn = posn;
// }

// // Clear all the entries in the
// // local symbol table
// void freelocsyms(void) {
//   Locls = NSYMBOLS - 1;
// }

// Add a global symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc
// + class of the symbol
// + size: number of elements
// + endlabel: if this is a function
// Return the slot number in the symbol table
struct symtable *addglob(char *name, int type, int stype, int class, int size) {
    struct symtable *sym = newsym(name, type, stype, class, size, 0);
    appendsym(&Globhead, &Globtail, sym);
    return sym;
}

// Add a local symbol to the symbol table. Set up its:
// + type: char, int etc.
// + structural type: var, function, array etc.
// + size: number of elements
// + isparam: if true, this is a parameter to the function
// Return the slot number in the symbol table, -1 if a duplicate entry
struct symtable *addlocl(char *name, int type, int stype, int class, int size) {
    struct symtable *sym = newsym(name, type, stype, class, size, 0);
    appendsym(&Loclhead, &Locltail, sym);
    return sym;
}

// Add a symbol to the parameter list
struct symtable *addparm(char *name, int type, int stype, int class, int size) {
    struct symtable *sym = newsym(name, type, stype, class, size, 0);
    appendsym(&Parmhead, &Parmtail, sym);
    return sym;
}

// Determine if the symbol s is in the symbol table.
// Return its slot position or -1 if not found
// Will firstly try to find if it is a local symbol
// then try to find if it is global symbol
struct symtable *findsymbol(char *s) {
    struct symtable *node;

    // Look for a parameter if we are in a function's body
    if (Functionid) {
        node = findsyminlist(s, Functionid->member);
        if (node)
            return node;
    }
    // Otherwise, try the local and global symbol lists
    node = findsyminlist(s, Loclhead);
    if (node)
        return node;
    return findsyminlist(s, Globhead);
}

struct symtable *findcomposite(char *s) {
    return findsyminlist(s, Comphead);
}

// Reset the contents of the symbol table
void clear_symtable(void) {
    Globhead = Globtail = NULL;
    Loclhead = Locltail = NULL;
    Parmhead = Parmtail = NULL;
    Comphead = Comptail = NULL;
}

// Clear all the entries in the local symbol table
void freeloclsyms(void) {
    Loclhead = Locltail = NULL;
    Parmhead = Parmtail = NULL;
    Functionid = NULL;
}

// // Given a function's slot number, copy the global parameters
// // from its prototype to be local parameters
// void copyfuncparams(int slot) {
//     int i, id = slot + 1;

//     for (i = 0; i < Symtable[slot]->nelems; i++, id++) {
//         addlocl(Symtable[id]->name, Symtable[id]->type, Symtable[id]->stype, 
//                 Symtable[id]->class, Symtable[id]->size);
//     }
// }

// // Reset the contents of the symbol table
// void clear_symtable(void) {
//     Globs = 0;
//     Locls = NSYMBOLS - 1;
// }
