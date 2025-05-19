#include "../../include/lib/std.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include "../../include/common/util.h"
#include "../../include/core/vm.h"
#include "../../include/core/codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char* name;
    PrismValue (*function)(PrismValue*, int);
} StdFunction;

// Forward declarations
PrismValue prism_std_print(PrismValue* args, int arg_count);
PrismValue prism_std_render(PrismValue* args, int arg_count);
PrismValue prism_std_input(PrismValue* args, int arg_count);
PrismValue prism_std_type(PrismValue* args, int arg_count);
PrismValue prism_std_string(PrismValue* args, int arg_count);
PrismValue prism_std_int(PrismValue* args, int arg_count);
PrismValue prism_std_float(PrismValue* args, int arg_count);
PrismValue prism_std_bool(PrismValue* args, int arg_count);

static StdFunction std_functions[] = {
    {"print", prism_std_print},
    {"render", prism_std_render},
    {"input", prism_std_input},
    {"type", prism_std_type},
    {"string", prism_std_string},
    {"int", prism_std_int},
    {"float", prism_std_float},
    {"bool", prism_std_bool},
    {NULL, NULL} // Sentinel
};

void prism_std_init() {
    // TODO: std init
}

void prism_std_cleanup() {
    // TODO: std cleanup
}

void prism_print_value(PrismValue value) {
    switch (value.type) {
        case TYPE_INT:
            printf("%ld", value.value.i);
            break;
        case TYPE_FLOAT:
            printf("%g", value.value.f);
            break;
        case TYPE_BOOL:
            printf("%s", value.value.b ? "true" : "false");
            break;
        case TYPE_STRING:
            printf("%s", value.value.s ? value.value.s : "");
            break;
        case TYPE_NONE:
            printf("nil");
            break;
        case TYPE_FUNCTION:
            printf("<function>");
            break;
        case TYPE_NATIVE:
            printf("<native function>");
            break;
        case TYPE_PRISM:
            printf("<prism>");
            break;
        default:
            printf("<unknown>");
            break;
    }
}

const char* prism_type_to_string(PrismType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        case TYPE_NONE: return "nil";
        case TYPE_FUNCTION: return "function";
        case TYPE_NATIVE: return "native";
        case TYPE_PRISM: return "prism";
        default: return "unknown";
    }
}

PrismValue prism_value_convert(PrismValue value, PrismType target_type) {
    PrismValue result;
    result.type = target_type;
    
    // Convert based on target type
    switch (target_type) {
        case TYPE_STRING:
            switch (value.type) {
                case TYPE_INT: {
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%ld", value.value.i);
                    result.value.s = strdup(buffer);
                    break;
                }
                case TYPE_FLOAT: {
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%g", value.value.f);
                    result.value.s = strdup(buffer);
                    break;
                }
                case TYPE_BOOL:
                    result.value.s = strdup(value.value.b ? "true" : "false");
                    break;
                case TYPE_STRING:
                    result.value.s = strdup(value.value.s);
                    break;
                default:
                    result.value.s = strdup("<unknown>");
                    break;
            }
            break;
            
        case TYPE_INT:
            switch (value.type) {
                case TYPE_INT:
                    result.value.i = value.value.i;
                    break;
                case TYPE_FLOAT:
                    result.value.i = (int64_t)value.value.f;
                    break;
                case TYPE_BOOL:
                    result.value.i = value.value.b ? 1 : 0;
                    break;
                case TYPE_STRING: {
                    char* end;
                    result.value.i = strtol(value.value.s, &end, 10);
                    if (*end != '\0') {
                        // Not a complete conversion
                        result.value.i = 0;
                    }
                    break;
                }
                default:
                    result.value.i = 0;
                    break;
            }
            break;
            
        case TYPE_FLOAT:
            switch (value.type) {
                case TYPE_INT:
                    result.value.f = (double)value.value.i;
                    break;
                case TYPE_FLOAT:
                    result.value.f = value.value.f;
                    break;
                case TYPE_BOOL:
                    result.value.f = value.value.b ? 1.0 : 0.0;
                    break;
                case TYPE_STRING: {
                    char* end;
                    result.value.f = strtod(value.value.s, &end);
                    if (*end != '\0') {
                        // Not a complete conversion
                        result.value.f = 0.0;
                    }
                    break;
                }
                default:
                    result.value.f = 0.0;
                    break;
            }
            break;
            
        case TYPE_BOOL:
            switch (value.type) {
                case TYPE_INT:
                    result.value.b = (value.value.i != 0);
                    break;
                case TYPE_FLOAT:
                    result.value.b = (value.value.f != 0.0);
                    break;
                case TYPE_BOOL:
                    result.value.b = value.value.b;
                    break;
                case TYPE_STRING:
                    // "true" or non-empty string is true
                    result.value.b = (strcmp(value.value.s, "true") == 0 || strlen(value.value.s) > 0);
                    break;
                default:
                    result.value.b = false;
                    break;
            }
            break;
            
        default:
            result.type = TYPE_NONE;
            break;
    }
    
    return result;
}

PrismValue prism_std_print(PrismValue* args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        prism_print_value(args[i]);
        if (i < arg_count - 1) {
            printf(" ");
        }
    }
    
    PrismValue result;
    result.type = TYPE_NONE;
    return result;
}

PrismValue prism_std_render(PrismValue* args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        prism_print_value(args[i]);
        if (i < arg_count - 1) {
            printf(" ");
        }
    }
    printf("\n");
    
    PrismValue result;
    result.type = TYPE_NONE;
    return result;
}

PrismValue prism_std_input(PrismValue* args, int arg_count) {
    // Optionally print a prompt
    if (arg_count > 0) {
        prism_print_value(args[0]);
    }
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup(buffer);
        return result;
    }
    
    PrismValue result;
    result.type = TYPE_STRING;
    result.value.s = strdup("");
    return result;
}

PrismValue prism_std_type(PrismValue* args, int arg_count) {
    if (arg_count < 1) {
        prism_error("Type function requires at least one argument");
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup("unknown");
        return result;
    }
    
    PrismValue result;
    result.type = TYPE_STRING;
    result.value.s = strdup(prism_type_to_string(args[0].type));
    return result;
}

PrismValue prism_std_string(PrismValue* args, int arg_count) {
    if (arg_count < 1) {
        prism_error("String conversion requires at least one argument");
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup("");
        return result;
    }
    
    return prism_value_convert(args[0], TYPE_STRING);
}

PrismValue prism_std_int(PrismValue* args, int arg_count) {
    if (arg_count < 1) {
        prism_error("Int conversion requires at least one argument");
        PrismValue result;
        result.type = TYPE_INT;
        result.value.i = 0;
        return result;
    }
    
    return prism_value_convert(args[0], TYPE_INT);
}

PrismValue prism_std_float(PrismValue* args, int arg_count) {
    if (arg_count < 1) {
        prism_error("Float conversion requires at least one argument");
        PrismValue result;
        result.type = TYPE_FLOAT;
        result.value.f = 0.0;
        return result;
    }
    
    return prism_value_convert(args[0], TYPE_FLOAT);
}

PrismValue prism_std_bool(PrismValue* args, int arg_count) {
    if (arg_count < 1) {
        prism_error("Bool conversion requires at least one argument");
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    return prism_value_convert(args[0], TYPE_BOOL);
}

void prism_std_register_all(void* vm_ptr) {
    VM* vm = (VM*)vm_ptr;
    if (!vm || !vm->code_gen) return;
    
    // Register all standard library functions
    for (int i = 0; std_functions[i].name != NULL; i++) {
        codegen_add_native_function(vm->code_gen, std_functions[i].name, std_functions[i].function);
    }
}