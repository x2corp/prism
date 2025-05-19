#include "../../include/common/types.h"
#include "../../include/common/memory.h"
#include <string.h>

const char* prism_type_to_string(PrismType type) {
    switch (type) {
        case TYPE_NONE: return "None";
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_FUNCTION: return "function";
        case TYPE_NATIVE: return "native_function";
        case TYPE_PRISM: return "prism";
        default: return "unknown";
    }
}

PrismType prism_type_from_string(const char* str) {
    if (strcmp(str, "None") == 0) return TYPE_NONE;
    if (strcmp(str, "int") == 0) return TYPE_INT;
    if (strcmp(str, "float") == 0) return TYPE_FLOAT;
    if (strcmp(str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(str, "string") == 0) return TYPE_STRING;
    if (strcmp(str, "function") == 0) return TYPE_FUNCTION;
    if (strcmp(str, "native_function") == 0) return TYPE_NATIVE;
    if (strcmp(str, "prism") == 0) return TYPE_PRISM;
    return TYPE_NONE; // Default
}

void prism_value_set(PrismValue* dest, PrismValue* src) {
    dest->type = src->type;
    
    switch (src->type) {
        case TYPE_INT:
            dest->value.i = src->value.i;
            break;
        case TYPE_FLOAT:
            dest->value.f = src->value.f;
            break;
        case TYPE_BOOL:
            dest->value.b = src->value.b;
            break;
        case TYPE_STRING:
            if (dest->value.s) prism_free(dest->value.s);
            dest->value.s = strdup(src->value.s);
            break;
        default:
            dest->value.ptr = src->value.ptr;
            break;
    }
}

PrismValue prism_value_convert(PrismValue value, PrismType target_type) {
    PrismValue result;
    result.type = target_type;
    
    switch (target_type) {
        case TYPE_INT:
            switch (value.type) {
                case TYPE_INT: result.value.i = value.value.i; break;
                case TYPE_FLOAT: result.value.i = (int64_t)value.value.f; break;
                case TYPE_BOOL: result.value.i = value.value.b ? 1 : 0; break;
                case TYPE_STRING: result.value.i = atoll(value.value.s); break;
                default: result.value.i = 0; break;
            }
            break;
        
        case TYPE_FLOAT:
            switch (value.type) {
                case TYPE_INT: result.value.f = (double)value.value.i; break;
                case TYPE_FLOAT: result.value.f = value.value.f; break;
                case TYPE_BOOL: result.value.f = value.value.b ? 1.0 : 0.0; break;
                case TYPE_STRING: result.value.f = atof(value.value.s); break;
                default: result.value.f = 0.0; break;
            }
            break;
        
        case TYPE_BOOL:
            switch (value.type) {
                case TYPE_INT: result.value.b = value.value.i != 0; break;
                case TYPE_FLOAT: result.value.b = value.value.f != 0.0; break;
                case TYPE_BOOL: result.value.b = value.value.b; break;
                case TYPE_STRING: result.value.b = strlen(value.value.s) > 0; break;
                default: result.value.b = false; break;
            }
            break;
        
        case TYPE_STRING: {
            char buffer[64];
            switch (value.type) {
                case TYPE_INT: 
                    snprintf(buffer, sizeof(buffer), "%lld", (long long)value.value.i);
                    result.value.s = strdup(buffer);
                    break;
                case TYPE_FLOAT:
                    snprintf(buffer, sizeof(buffer), "%g", value.value.f);
                    result.value.s = strdup(buffer);
                    break;
                case TYPE_BOOL:
                    result.value.s = strdup(value.value.b ? "true" : "false");
                    break;
                case TYPE_STRING:
                    result.value.s = strdup(value.value.s);
                    break;
                default:
                    result.value.s = strdup("");
                    break;
            }
            break;
        }
        
        default:
            result.value = value.value;
            break;
    }
    
    return result;
}