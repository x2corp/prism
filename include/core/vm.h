#ifndef PRISM_VM_H
#define PRISM_VM_H

#include "codegen.h"

#define STACK_MAX 256

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

typedef struct {
    CodeGenerator* code_gen;
    PrismValue stack[STACK_MAX];
    int stack_top;
    int frames[STACK_MAX];
    int frame_count;
} VM;

VM* vm_create();
void vm_free(VM* vm);
InterpretResult vm_interpret(VM* vm, const char* source, const char* filename);
void vm_push(VM* vm, PrismValue value);
PrismValue vm_pop(VM* vm);
PrismValue vm_peek(VM* vm, int distance);

#endif /* PRISM_VM_H */