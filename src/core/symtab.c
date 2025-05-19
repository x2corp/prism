#include "../../include/core/symtab.h"
#include "../../include/common/memory.h"
#include <string.h>

SymbolTable* symtab_create() {
    SymbolTable* table = prism_alloc(sizeof(SymbolTable));
    table->current = prism_alloc(sizeof(Scope));
    table->current->entries = NULL;
    table->current->parent = NULL;
    return table;
}

static void free_scope(Scope* scope) {
    SymbolEntry* entry = scope->entries;
    while (entry) {
        SymbolEntry* next = entry->next;
        prism_free(entry->name);
        prism_free(entry);
        entry = next;
    }
    prism_free(scope);
}

void symtab_free(SymbolTable* table) {
    if (!table) return;
    
    Scope* scope = table->current;
    while (scope) {
        Scope* parent = scope->parent;
        free_scope(scope);
        scope = parent;
    }
    
    prism_free(table);
}

void symtab_enter_scope(SymbolTable* table) {
    Scope* scope = prism_alloc(sizeof(Scope));
    scope->entries = NULL;
    scope->parent = table->current;
    table->current = scope;
}

void symtab_exit_scope(SymbolTable* table) {
    if (!table->current->parent) return; // Can't exit global scope
    
    Scope* old = table->current;
    table->current = old->parent;
    free_scope(old);
}

void symtab_define(SymbolTable* table, const char* name, PrismType type, bool exposed, bool internal, void* value) {
    SymbolEntry* entry = prism_alloc(sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->type = type;
    entry->exposed = exposed;
    entry->internal = internal;
    entry->data = value;
    entry->next = table->current->entries;
    table->current->entries = entry;
}

SymbolEntry* symtab_lookup_current(SymbolTable* table, const char* name) {
    SymbolEntry* entry = table->current->entries;
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

SymbolEntry* symtab_lookup(SymbolTable* table, const char* name) {
    Scope* scope = table->current;
    while (scope) {
        SymbolEntry* entry = scope->entries;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }
    return NULL;
}