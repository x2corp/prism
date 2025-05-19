#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <string.h>

void* prism_alloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        prism_error("Memory allocation failed");
        exit(1);
    }
    memset(ptr, 0, size);
    return ptr;
}

void* prism_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        prism_error("Memory reallocation failed");
        exit(1);
    }
    return new_ptr;
}

void prism_free(void* ptr) {
    if (ptr) free(ptr);
}

PrismValue* prism_value_create(PrismType type) {
    PrismValue* value = (PrismValue*)prism_alloc(sizeof(PrismValue));
    value->type = type;
    return value;
}

void prism_value_free(PrismValue* value) {
    if (!value) return;
    
    if (value->type == TYPE_STRING && value->value.s) {
        prism_free(value->value.s);
    }
    
    prism_free(value);
}

PrismVariable* prism_var_create(char* name, PrismType type, bool exposed, bool internal) {
    PrismVariable* var = (PrismVariable*)prism_alloc(sizeof(PrismVariable));
    var->name = strdup(name);
    var->type = type;
    var->exposed = exposed;
    var->internal = internal;
    var->value.type = type;
    return var;
}

void prism_var_free(PrismVariable* var) {
    if (!var) return;
    
    if (var->name) prism_free(var->name);
    if (var->value.type == TYPE_STRING && var->value.value.s) {
        prism_free(var->value.value.s);
    }
    
    prism_free(var);
}