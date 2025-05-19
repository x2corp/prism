#include "../../include/lib/io.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include "../../include/common/util.h"
#include "../../include/core/vm.h"
#include "../../include/core/codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    const char* name;
    PrismValue (*function)(PrismValue*, int);
} IoFunction;

static IoFunction io_functions[] = {
    {"read_file", prism_io_read_file},
    {"write_file", prism_io_write_file},
    {"append_file", prism_io_append_file},
    {"file_exists", prism_io_file_exists},
    {"delete_file", prism_io_delete_file},
    {NULL, NULL} // Sentinel
};

void prism_io_init() {
    // TODO: io init
};

void prism_io_cleanup() {
    // TODO: io cleanup
};

PrismValue prism_io_read_file(PrismValue* args, int arg_count) {
    if (arg_count < 1 || args[0].type != TYPE_STRING) {
        prism_error("read_file requires a string filename argument");
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup("");
        return result;
    }
    
    FILE* file = fopen(args[0].value.s, "r");
    if (!file) {
        prism_error("Could not open file '%s' for reading", args[0].value.s);
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup("");
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    
    // Allocate buffer
    char* buffer = prism_alloc(size + 1);
    if (!buffer) {
        fclose(file);
        prism_error("Not enough memory to read file '%s'", args[0].value.s);
        PrismValue result;
        result.type = TYPE_STRING;
        result.value.s = strdup("");
        return result;
    }
    
    // Read file content
    size_t read_size = fread(buffer, 1, size, file);
    buffer[read_size] = '\0';
    fclose(file);
    
    PrismValue result;
    result.type = TYPE_STRING;
    result.value.s = buffer;
    return result;
}

PrismValue prism_io_write_file(PrismValue* args, int arg_count) {
    if (arg_count < 2 || args[0].type != TYPE_STRING || args[1].type != TYPE_STRING) {
        prism_error("write_file requires string filename and content arguments");
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    FILE* file = fopen(args[0].value.s, "w");
    if (!file) {
        prism_error("Could not open file '%s' for writing", args[0].value.s);
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    fputs(args[1].value.s, file);
    fclose(file);
    
    PrismValue result;
    result.type = TYPE_BOOL;
    result.value.b = true;
    return result;
}

PrismValue prism_io_append_file(PrismValue* args, int arg_count) {
    if (arg_count < 2 || args[0].type != TYPE_STRING || args[1].type != TYPE_STRING) {
        prism_error("append_file requires string filename and content arguments");
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    FILE* file = fopen(args[0].value.s, "a");
    if (!file) {
        prism_error("Could not open file '%s' for appending", args[0].value.s);
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    fputs(args[1].value.s, file);
    fclose(file);
    
    PrismValue result;
    result.type = TYPE_BOOL;
    result.value.b = true;
    return result;
}

PrismValue prism_io_file_exists(PrismValue* args, int arg_count) {
    if (arg_count < 1 || args[0].type != TYPE_STRING) {
        prism_error("file_exists requires a string filename argument");
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    struct stat buffer;
    bool exists = (stat(args[0].value.s, &buffer) == 0);
    
    PrismValue result;
    result.type = TYPE_BOOL;
    result.value.b = exists;
    return result;
}

PrismValue prism_io_delete_file(PrismValue* args, int arg_count) {
    if (arg_count < 1 || args[0].type != TYPE_STRING) {
        prism_error("delete_file requires a string filename argument");
        PrismValue result;
        result.type = TYPE_BOOL;
        result.value.b = false;
        return result;
    }
    
    int status = unlink(args[0].value.s);
    
    PrismValue result;
    result.type = TYPE_BOOL;
    result.value.b = (status == 0);
    return result;
}

void prism_io_register_all(void* vm_ptr) {
    VM* vm = (VM*)vm_ptr;
    if (!vm || !vm->code_gen) return;
    
    for (int i = 0; io_functions[i].name != NULL; i++) {
        codegen_add_native_function(vm->code_gen, io_functions[i].name, io_functions[i].function);
    }
}