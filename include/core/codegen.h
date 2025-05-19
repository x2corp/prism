#ifndef PRISM_CODEGEN_H
#define PRISM_CODEGEN_H

#include "ast.h"
#include "symtab.h"

typedef enum {
    OP_NOP,
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN,
    OP_CALL,
    OP_LOAD,
    OP_STORE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_POP
} OpCode;

typedef struct {
    OpCode* code;
    int* lines;
    int count;
    int capacity;
   
    PrismValue* constants;
    int constant_count;
    int constant_capacity;
} CodeChunk;

// Define the NativeFunction structure
typedef struct {
    char* name;
    PrismValue (*function)(PrismValue*, int);
} NativeFunction;

typedef struct {
    CodeChunk* chunks;
    int chunk_count;
    SymbolTable* symtab;
    
    // Add native function support
    NativeFunction* natives;
    int native_count;
    int native_capacity;
} CodeGenerator;

CodeGenerator* codegen_create();
void codegen_free(CodeGenerator* generator);
int codegen_emit_constant(CodeGenerator* generator, PrismValue value);
void codegen_emit_byte(CodeGenerator* generator, OpCode op, int line);
int codegen_emit_jump(CodeGenerator* generator, OpCode op, int line);
void codegen_patch_jump(CodeGenerator* generator, int offset);

// Add function declarations for native function handling
void codegen_add_native_function(CodeGenerator* generator, const char* name, PrismValue (*function)(PrismValue*, int));
NativeFunction* codegen_get_native_function(CodeGenerator* generator, int index);

void codegen_generate(CodeGenerator* generator, Program* program);

#endif /* PRISM_CODEGEN_H */