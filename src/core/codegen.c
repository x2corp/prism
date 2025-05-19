#include "../../include/core/codegen.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <string.h>

#define INITIAL_CHUNK_CAPACITY 64
#define INITIAL_CONSTANT_CAPACITY 16
#define INITIAL_NATIVE_CAPACITY 16

static CodeChunk* current_chunk;

CodeGenerator* codegen_create() {
    CodeGenerator* generator = prism_alloc(sizeof(CodeGenerator));
    generator->chunks = prism_alloc(sizeof(CodeChunk));
    generator->chunk_count = 1;
    
    generator->chunks[0].code = prism_alloc(sizeof(OpCode) * INITIAL_CHUNK_CAPACITY);
    generator->chunks[0].lines = prism_alloc(sizeof(int) * INITIAL_CHUNK_CAPACITY);
    generator->chunks[0].count = 0;
    generator->chunks[0].capacity = INITIAL_CHUNK_CAPACITY;
    
    generator->chunks[0].constants = prism_alloc(sizeof(PrismValue) * INITIAL_CONSTANT_CAPACITY);
    generator->chunks[0].constant_count = 0;
    generator->chunks[0].constant_capacity = INITIAL_CONSTANT_CAPACITY;
    
    generator->symtab = symtab_create();
    
    // Initialize native function table
    generator->natives = prism_alloc(sizeof(NativeFunction) * INITIAL_NATIVE_CAPACITY);
    generator->native_count = 0;
    generator->native_capacity = INITIAL_NATIVE_CAPACITY;
    
    current_chunk = &generator->chunks[0];
    
    return generator;
}

void codegen_free(CodeGenerator* generator) {
    if (!generator) return;
    
    for (int i = 0; i < generator->chunk_count; i++) {
        CodeChunk* chunk = &generator->chunks[i];
        prism_free(chunk->code);
        prism_free(chunk->lines);
        
        for (int j = 0; j < chunk->constant_count; j++) {
            if (chunk->constants[j].type == TYPE_STRING && chunk->constants[j].value.s) {
                prism_free(chunk->constants[j].value.s);
            }
        }
        
        prism_free(chunk->constants);
    }
    
    // Free native function names
    for (int i = 0; i < generator->native_count; i++) {
        prism_free(generator->natives[i].name);
    }
    prism_free(generator->natives);
    
    prism_free(generator->chunks);
    symtab_free(generator->symtab);
    prism_free(generator);
}

int codegen_emit_constant(CodeGenerator* generator, PrismValue value) {
    if (current_chunk->constant_count >= current_chunk->constant_capacity) {
        current_chunk->constant_capacity *= 2;
        current_chunk->constants = prism_realloc(current_chunk->constants, 
                                                sizeof(PrismValue) * current_chunk->constant_capacity);
    }
    
    // Copy the constant value
    current_chunk->constants[current_chunk->constant_count].type = value.type;
    switch (value.type) {
        case TYPE_INT:
            current_chunk->constants[current_chunk->constant_count].value.i = value.value.i;
            break;
        case TYPE_FLOAT:
            current_chunk->constants[current_chunk->constant_count].value.f = value.value.f;
            break;
        case TYPE_BOOL:
            current_chunk->constants[current_chunk->constant_count].value.b = value.value.b;
            break;
        case TYPE_STRING:
            current_chunk->constants[current_chunk->constant_count].value.s = strdup(value.value.s);
            break;
        default:
            break;
    }
    
    return current_chunk->constant_count++;
}

void codegen_emit_byte(CodeGenerator* generator, OpCode op, int line) {
    if (current_chunk->count >= current_chunk->capacity) {
        current_chunk->capacity *= 2;
        current_chunk->code = prism_realloc(current_chunk->code, 
                                           sizeof(OpCode) * current_chunk->capacity);
        current_chunk->lines = prism_realloc(current_chunk->lines, 
                                            sizeof(int) * current_chunk->capacity);
    }
    
    current_chunk->code[current_chunk->count] = op;
    current_chunk->lines[current_chunk->count] = line;
    current_chunk->count++;
}

int codegen_emit_jump(CodeGenerator* generator, OpCode op, int line) {
    codegen_emit_byte(generator, op, line);
    codegen_emit_byte(generator, 0xFF, line); // Placeholder for jump offset
    codegen_emit_byte(generator, 0xFF, line);
    return current_chunk->count - 2;
}

void codegen_patch_jump(CodeGenerator* generator, int offset) {
    // -2 to adjust for the jump offset itself
    int jump = current_chunk->count - offset - 2;
    
    if (jump > 0xFFFF) {
        prism_error("Jump offset too large");
    }
    
    current_chunk->code[offset] = (jump >> 8) & 0xFF;
    current_chunk->code[offset + 1] = jump & 0xFF;
}

void codegen_add_native_function(CodeGenerator* generator, const char* name, PrismValue (*function)(PrismValue*, int)) {
    if (!generator) return;

    if (generator->native_count >= generator->native_capacity) {
        generator->native_capacity *= 2;
        generator->natives = prism_realloc(generator->natives, 
                                          sizeof(NativeFunction) * generator->native_capacity);
    }
    
    // Add the native function
    generator->natives[generator->native_count].name = strdup(name);
    generator->natives[generator->native_count].function = function;
    
    // Add the function to the symbol table - use the native function index as the "special" value
    // The negative index indicates it's a native function
    symtab_define(generator->symtab, name, TYPE_FUNCTION, true, false, 
                 (void*)(intptr_t)(-(generator->native_count + 1)));
    
    generator->native_count++;
}

// Lookup a native function by index (the index is expected to be negative)
NativeFunction* codegen_get_native_function(CodeGenerator* generator, int index) {
    if (!generator) return NULL;
    
    // Convert from the negative index stored in the symbol table to zero-based index
    int real_index = -index - 1;
    
    if (real_index < 0 || real_index >= generator->native_count) {
        return NULL;
    }
    
    return &generator->natives[real_index];
}

static void generate_expr(CodeGenerator* generator, Expr* expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case EXPR_LITERAL: {
            PrismValue value = expr->as.literal;
            int constant = codegen_emit_constant(generator, value);
            codegen_emit_byte(generator, OP_CONSTANT, 0);
            codegen_emit_byte(generator, constant, 0);
            break;
        }
        case EXPR_VARIABLE: {
            SymbolEntry* entry = symtab_lookup(generator->symtab, expr->as.variable.name);
            if (!entry) {
                prism_error("Undefined variable '%s'", expr->as.variable.name);
                return;
            }
            
            // Handle function references differently
            if (entry->type == TYPE_FUNCTION) {
                // Push the function index (positive for bytecode functions, negative for native functions)
                PrismValue func_idx;
                func_idx.type = TYPE_FUNCTION;
                func_idx.value.i = (int)(intptr_t)entry->data;
                
                int constant = codegen_emit_constant(generator, func_idx);
                codegen_emit_byte(generator, OP_CONSTANT, 0);
                codegen_emit_byte(generator, constant, 0);
            } else {
                // Load the variable value
                codegen_emit_byte(generator, OP_LOAD, 0);
                // TODO: Implement variable index lookup
                codegen_emit_byte(generator, (int)(intptr_t)entry->data, 0);
            }
            break;
        }
        case EXPR_CALL: {
            // Generate code for the arguments
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                generate_expr(generator, expr->as.call.args[i]);
            }
            
            // Generate code for the function itself
            generate_expr(generator, expr->as.call.callee);
            
            // Emit the call instruction with the argument count
            codegen_emit_byte(generator, OP_CALL, 0);
            codegen_emit_byte(generator, expr->as.call.arg_count, 0);
            break;
        }
        case EXPR_BINARY: {
            generate_expr(generator, expr->as.binary.left);
            generate_expr(generator, expr->as.binary.right);
            
            // Determine the operation
            if (strcmp(expr->as.binary.op, "+") == 0) {
                codegen_emit_byte(generator, OP_ADD, 0);
            } else if (strcmp(expr->as.binary.op, "-") == 0) {
                codegen_emit_byte(generator, OP_SUBTRACT, 0);
            } else if (strcmp(expr->as.binary.op, "*") == 0) {
                codegen_emit_byte(generator, OP_MULTIPLY, 0);
            } else if (strcmp(expr->as.binary.op, "/") == 0) {
                codegen_emit_byte(generator, OP_DIVIDE, 0);
            } else {
                prism_error("Unknown binary operator '%s'", expr->as.binary.op);
            }
            break;
        }
        case EXPR_UNARY: {
            generate_expr(generator, expr->as.unary.operand);
            
            // Determine the operation
            if (strcmp(expr->as.unary.op, "-") == 0) {
                codegen_emit_byte(generator, OP_NEGATE, 0);
            } else {
                prism_error("Unknown unary operator '%s'", expr->as.unary.op);
            }
            break;
        }
    }
}

static void generate_stmt(CodeGenerator* generator, Stmt* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case STMT_EXPR:
            generate_expr(generator, stmt->as.expr);
            codegen_emit_byte(generator, OP_POP, 0); // Pop the result
            break;
            
        case STMT_VAR_DECL: {
            // Generate the initializer expression
            if (stmt->as.var_decl.initializer) {
                generate_expr(generator, stmt->as.var_decl.initializer);
            } else {
                // Default initialization to nil if no initializer
                PrismValue nil;
                nil.type = TYPE_NONE;
                int constant = codegen_emit_constant(generator, nil);
                codegen_emit_byte(generator, OP_CONSTANT, 0);
                codegen_emit_byte(generator, constant, 0);
            }
            
            // Add the variable to the symbol table
            int var_index = generator->symtab->current_vars++;
            symtab_define(generator->symtab, stmt->as.var_decl.name, 
                         stmt->as.var_decl.type, stmt->as.var_decl.exposed, 
                         stmt->as.var_decl.internal, (void*)(intptr_t)var_index);
            
            // Emit the store instruction
            codegen_emit_byte(generator, OP_STORE, 0);
            codegen_emit_byte(generator, var_index, 0);
            break;
        }
            
        case STMT_FUNC_DECL: {
            // Create a new chunk for the function
            int func_chunk_idx = generator->chunk_count++;
            generator->chunks = prism_realloc(generator->chunks, 
                                             sizeof(CodeChunk) * generator->chunk_count);
            
            CodeChunk* old_chunk = current_chunk;
            current_chunk = &generator->chunks[func_chunk_idx];
            
            current_chunk->code = prism_alloc(sizeof(OpCode) * INITIAL_CHUNK_CAPACITY);
            current_chunk->lines = prism_alloc(sizeof(int) * INITIAL_CHUNK_CAPACITY);
            current_chunk->count = 0;
            current_chunk->capacity = INITIAL_CHUNK_CAPACITY;
            
            current_chunk->constants = prism_alloc(sizeof(PrismValue) * INITIAL_CONSTANT_CAPACITY);
            current_chunk->constant_count = 0;
            current_chunk->constant_capacity = INITIAL_CONSTANT_CAPACITY;
            
            // Define the function in the symbol table
            symtab_define(generator->symtab, stmt->as.func_decl.name, 
                         TYPE_FUNCTION, false, false, (void*)(intptr_t)func_chunk_idx);
            
            // Enter a new scope for the function
            symtab_enter_scope(generator->symtab);
            
            // Define parameters
            for (int i = 0; i < stmt->as.func_decl.param_count; i++) {
                int param_index = generator->symtab->current_vars++;
                symtab_define(generator->symtab, stmt->as.func_decl.params[i], 
                             stmt->as.func_decl.param_types[i], false, false, 
                             (void*)(intptr_t)param_index);
            }
            
            // Generate code for the function body
            for (int i = 0; i < stmt->as.func_decl.body_count; i++) {
                generate_stmt(generator, stmt->as.func_decl.body[i]);
            }
            
            // Add return if one wasn't explicitly given
            if (current_chunk->count == 0 || current_chunk->code[current_chunk->count - 1] != OP_RETURN) {
                PrismValue nil;
                nil.type = TYPE_NONE;
                int constant = codegen_emit_constant(generator, nil);
                codegen_emit_byte(generator, OP_CONSTANT, 0);
                codegen_emit_byte(generator, constant, 0);
                codegen_emit_byte(generator, OP_RETURN, 0);
            }
            
            // Exit the function scope
            symtab_exit_scope(generator->symtab);
            
            // Restore the previous chunk
            current_chunk = old_chunk;
            break;
        }
            
        case STMT_PRISM_DECL: {
            // Similar to function declaration, but with additional prism-specific behavior
            int prism_chunk_idx = generator->chunk_count++;
            generator->chunks = prism_realloc(generator->chunks, 
                                             sizeof(CodeChunk) * generator->chunk_count);
            
            CodeChunk* old_chunk = current_chunk;
            current_chunk = &generator->chunks[prism_chunk_idx];
            
            current_chunk->code = prism_alloc(sizeof(OpCode) * INITIAL_CHUNK_CAPACITY);
            current_chunk->lines = prism_alloc(sizeof(int) * INITIAL_CHUNK_CAPACITY);
            current_chunk->count = 0;
            current_chunk->capacity = INITIAL_CHUNK_CAPACITY;
            
            current_chunk->constants = prism_alloc(sizeof(PrismValue) * INITIAL_CONSTANT_CAPACITY);
            current_chunk->constant_count = 0;
            current_chunk->constant_capacity = INITIAL_CONSTANT_CAPACITY;
            
            // Define the prism in the symbol table
            symtab_define(generator->symtab, stmt->as.prism_decl.name, 
                         TYPE_PRISM, false, false, (void*)(intptr_t)prism_chunk_idx);
            
            // Enter a new scope for the prism
            symtab_enter_scope(generator->symtab);
            
            // Generate code for the prism body
            for (int i = 0; i < stmt->as.prism_decl.body_count; i++) {
                generate_stmt(generator, stmt->as.prism_decl.body[i]);
            }
            
            // Add return if one wasn't explicitly given
            if (current_chunk->count == 0 || current_chunk->code[current_chunk->count - 1] != OP_RETURN) {
                PrismValue nil;
                nil.type = TYPE_NONE;
                int constant = codegen_emit_constant(generator, nil);
                codegen_emit_byte(generator, OP_CONSTANT, 0);
                codegen_emit_byte(generator, constant, 0);
                codegen_emit_byte(generator, OP_RETURN, 0);
            }
            
            // Exit the prism scope
            symtab_exit_scope(generator->symtab);
            
            // Restore the previous chunk
            current_chunk = old_chunk;
            break;
        }
            
        case STMT_RETURN:
            if (stmt->as.return_stmt.value) {
                generate_expr(generator, stmt->as.return_stmt.value);
            } else {
                // Default return value is nil
                PrismValue nil;
                nil.type = TYPE_NONE;
                int constant = codegen_emit_constant(generator, nil);
                codegen_emit_byte(generator, OP_CONSTANT, 0);
                codegen_emit_byte(generator, constant, 0);
            }
            
            codegen_emit_byte(generator, OP_RETURN, 0);
            break;
            
        case STMT_CALL:
            // Generate code for the arguments
            for (int i = 0; i < stmt->as.call.arg_count; i++) {
                generate_expr(generator, stmt->as.call.args[i]);
            }
            
            // Generate code for the function itself
            generate_expr(generator, stmt->as.call.callee);
            
            // Emit the call instruction with the argument count
            codegen_emit_byte(generator, OP_CALL, 0);
            codegen_emit_byte(generator, stmt->as.call.arg_count, 0);
            
            // Pop the result since this is a statement
            codegen_emit_byte(generator, OP_POP, 0);
            break;
    }
}

void codegen_generate(CodeGenerator* generator, Program* program) {
    // Reset the current chunk
    current_chunk = &generator->chunks[0];
    
    // Generate code for each statement
    for (int i = 0; i < program->count; i++) {
        generate_stmt(generator, program->statements[i]);
    }
    
    // Add a final return if one wasn't provided
    if (current_chunk->count == 0 || current_chunk->code[current_chunk->count - 1] != OP_RETURN) {
        PrismValue nil;
        nil.type = TYPE_NONE;
        int constant = codegen_emit_constant(generator, nil);
        codegen_emit_byte(generator, OP_CONSTANT, 0);
        codegen_emit_byte(generator, constant, 0);
        codegen_emit_byte(generator, OP_RETURN, 0);
    }
}