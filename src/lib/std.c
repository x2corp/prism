#include "../../include/lib/std.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include "../../include/common/types.h"
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