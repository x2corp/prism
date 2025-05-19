#include "../../include/core/vm.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <stdio.h>
#include <string.h>

VM* vm_create() {
    VM* vm = prism_alloc(sizeof(VM));
    vm->code_gen = NULL;
    vm->stack_top = 0;
    vm->frame_count = 0;
    return vm;
}

void vm_free(VM* vm) {
    if (!vm) return;
    
    if (vm->code_gen) {
        codegen_free(vm->code_gen);
    }
    
    prism_free(vm);
}

void vm_push(VM* vm, PrismValue value) {
    if (vm->stack_top >= STACK_MAX) {
        prism_error("Stack overflow");
        return;
    }
    
    // Copy value to stack
    vm->stack[vm->stack_top] = value;
    vm->stack_top++;
}

PrismValue vm_pop(VM* vm) {
    if (vm->stack_top <= 0) {
        prism_error("Stack underflow");
        PrismValue error_val;
        error_val.type = TYPE_NONE;
        return error_val;
    }
    
    vm->stack_top--;
    return vm->stack[vm->stack_top];
}

PrismValue vm_peek(VM* vm, int distance) {
    if (vm->stack_top <= distance) {
        prism_error("Stack underflow on peek");
        PrismValue error_val;
        error_val.type = TYPE_NONE;
        return error_val;
    }
    
    return vm->stack[vm->stack_top - 1 - distance];
}

static InterpretResult run(VM* vm) {
    CodeChunk* chunk = &vm->code_gen->chunks[0];
    int ip = 0;
    
    #define READ_BYTE() (chunk->code[ip++])
    #define READ_CONSTANT() (chunk->constants[READ_BYTE()])
    
    for (;;) {
        OpCode instruction = READ_BYTE();
        
        switch (instruction) {
            case OP_NOP:
                break;
                
            case OP_CONSTANT: {
                PrismValue constant = READ_CONSTANT();
                vm_push(vm, constant);
                break;
            }
            
            case OP_ADD: {
                if (vm->stack_top < 2) {
                    prism_error("Not enough operands for addition");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                PrismValue b = vm_pop(vm);
                PrismValue a = vm_pop(vm);
                
                if (a.type == TYPE_INT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_INT;
                    result.value.i = a.value.i + b.value.i;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f + b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_INT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = (double)a.value.i + b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f + (double)b.value.i;
                    vm_push(vm, result);
                } else if (a.type == TYPE_STRING && b.type == TYPE_STRING) {
                    PrismValue result;
                    result.type = TYPE_STRING;
                    
                    size_t len_a = strlen(a.value.s);
                    size_t len_b = strlen(b.value.s);
                    
                    result.value.s = prism_alloc(len_a + len_b + 1);
                    strcpy(result.value.s, a.value.s);
                    strcat(result.value.s, b.value.s);
                    
                    vm_push(vm, result);
                } else {
                    prism_error("Invalid operand types for addition");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            
            case OP_SUBTRACT: {
                if (vm->stack_top < 2) {
                    prism_error("Not enough operands for subtraction");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                PrismValue b = vm_pop(vm);
                PrismValue a = vm_pop(vm);
                
                if (a.type == TYPE_INT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_INT;
                    result.value.i = a.value.i - b.value.i;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f - b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_INT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = (double)a.value.i - b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f - (double)b.value.i;
                    vm_push(vm, result);
                } else {
                    prism_error("Invalid operand types for subtraction");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            
            case OP_MULTIPLY: {
                if (vm->stack_top < 2) {
                    prism_error("Not enough operands for multiplication");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                PrismValue b = vm_pop(vm);
                PrismValue a = vm_pop(vm);
                
                if (a.type == TYPE_INT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_INT;
                    result.value.i = a.value.i * b.value.i;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f * b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_INT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = (double)a.value.i * b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f * (double)b.value.i;
                    vm_push(vm, result);
                } else {
                    prism_error("Invalid operand types for multiplication");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            
            case OP_DIVIDE: {
                if (vm->stack_top < 2) {
                    prism_error("Not enough operands for division");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                PrismValue b = vm_pop(vm);
                PrismValue a = vm_pop(vm);
                
                if ((b.type == TYPE_INT && b.value.i == 0) ||
                    (b.type == TYPE_FLOAT && b.value.f == 0.0)) {
                    prism_error("Division by zero");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                if (a.type == TYPE_INT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = (double)a.value.i / (double)b.value.i;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f / b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_INT && b.type == TYPE_FLOAT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = (double)a.value.i / b.value.f;
                    vm_push(vm, result);
                } else if (a.type == TYPE_FLOAT && b.type == TYPE_INT) {
                    PrismValue result;
                    result.type = TYPE_FLOAT;
                    result.value.f = a.value.f / (double)b.value.i;
                    vm_push(vm, result);
                } else {
                    prism_error("Invalid operand types for division");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            
            case OP_NEGATE: {
                if (vm->stack_top < 1) {
                    prism_error("Not enough operands for negation");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                PrismValue operand = vm_pop(vm);
                PrismValue result;
                
                if (operand.type == TYPE_INT) {
                    result.type = TYPE_INT;
                    result.value.i = -operand.value.i;
                } else if (operand.type == TYPE_FLOAT) {
                    result.type = TYPE_FLOAT;
                    result.value.f = -operand.value.f;
                } else {
                    prism_error("Can only negate numbers");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                vm_push(vm, result);
                break;
            }
            
            case OP_RETURN: {
                if (vm->frame_count == 0) {
                    // Exit the program
                    return INTERPRET_OK;
                }
                
                // Return from function
                vm->frame_count--;
                ip = vm->frames[vm->frame_count];
                break;
            }
            
            case OP_CALL: {
                int arg_count = READ_BYTE();
                PrismValue callee = vm_peek(vm, arg_count);
                
                // Save current IP
                vm->frames[vm->frame_count++] = ip;
                
                // Jump to function
                ip = (int)callee.value.i;
                break;
            }
            
            case OP_LOAD: {
                int index = READ_BYTE();
                // TODO: symbol lookup
                break;
            }
            
            case OP_STORE: {
                int index = READ_BYTE();
                // TODO: symbol storage
                break;
            }
            
            case OP_JUMP: {
                int offset = READ_BYTE();
                ip += offset;
                break;
            }
            
            case OP_JUMP_IF_FALSE: {
                int offset = READ_BYTE();
                PrismValue condition = vm_pop(vm);
                
                if ((condition.type == TYPE_BOOL && !condition.value.b) ||
                    (condition.type == TYPE_INT && condition.value.i == 0) ||
                    (condition.type == TYPE_FLOAT && condition.value.f == 0.0) ||
                    (condition.type == TYPE_NONE)) {
                    ip += offset;
                }
                break;
            }
            
            case OP_POP: {
                vm_pop(vm);
                break;
            }
        }
        
        if (ip >= chunk->count) {
            return INTERPRET_OK;
        }
    }
    
    #undef READ_BYTE
    #undef READ_CONSTANT
    
    return INTERPRET_OK;
}

InterpretResult vm_interpret(VM* vm, const char* source, const char* filename) {
    // Create lexer
    Lexer* lexer = lexer_create(source, filename);
    lexer_scan_tokens(lexer);
    
    int token_count;
    Token* tokens = lexer_get_tokens(lexer, &token_count);
    
    if (prism_get_last_error()->type != ERROR_NONE) {
        lexer_free(lexer);
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Parse tokens
    Parser* parser = parser_create(tokens, token_count);
    Program* program = parser_parse(parser);
    
    if (prism_get_last_error()->type != ERROR_NONE) {
        parser_free(parser);
        lexer_free(lexer);
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Generate code
    vm->code_gen = codegen_create();
    codegen_generate(vm->code_gen, program);
    
    if (prism_get_last_error()->type != ERROR_NONE) {
        ast_free_program(program);
        parser_free(parser);
        lexer_free(lexer);
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Run the bytecode
    InterpretResult result = run(vm);
    
    // Cleanup
    ast_free_program(program);
    parser_free(parser);
    lexer_free(lexer);
    
    return result;
}