#ifndef PRISM_SYMTAB_H
#define PRISM_SYMTAB_H

#include "../common/types.h"

typedef struct SymbolEntry {
    char* name;
    PrismType type;
    bool exposed;
    bool internal;
    void* data;
    struct SymbolEntry* next;
} SymbolEntry;

typedef struct Scope {
    SymbolEntry* entries;
    struct Scope* parent;
} Scope;

typedef struct {
    Scope* current;
    int current_vars;
} SymbolTable;

SymbolTable* symtab_create();
void symtab_free(SymbolTable* table);
void symtab_enter_scope(SymbolTable* table);
void symtab_exit_scope(SymbolTable* table);
void symtab_define(SymbolTable* table, const char* name, PrismType type, bool exposed, bool internal, void* value);
SymbolEntry* symtab_lookup(SymbolTable* table, const char* name);
SymbolEntry* symtab_lookup_current(SymbolTable* table, const char* name);

#endif /* PRISM_SYMTAB_H */