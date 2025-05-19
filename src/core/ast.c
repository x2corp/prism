#include "../../include/core/ast.h"
#include "../../include/common/memory.h"
#include <string.h>

#define INITIAL_PROGRAM_CAPACITY 16

Program* ast_create_program() {
    Program* program = prism_alloc(sizeof(Program));
    program->count = 0;
    program->capacity = INITIAL_PROGRAM_CAPACITY;
    program->statements = prism_alloc(sizeof(Stmt*) * program->capacity);
    return program;
}

void ast_free_program(Program* program) {
    if (!program) return;
    
    for (int i = 0; i < program->count; i++) {
        ast_free_stmt(program->statements[i]);
    }
    
    prism_free(program->statements);
    prism_free(program);
}

void ast_add_statement(Program* program, Stmt* stmt) {
    if (program->count >= program->capacity) {
        program->capacity *= 2;
        program->statements = prism_realloc(program->statements, sizeof(Stmt*) * program->capacity);
    }
    
    program->statements[program->count++] = stmt;
}

Expr* ast_create_literal_expr(PrismValue value) {
    Expr* expr = prism_alloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    
    // Copy literal value
    expr->as.literal.type = value.type;
    switch (value.type) {
        case TYPE_INT:
            expr->as.literal.value.i = value.value.i;
            break;
        case TYPE_FLOAT:
            expr->as.literal.value.f = value.value.f;
            break;
        case TYPE_BOOL:
            expr->as.literal.value.b = value.value.b;
            break;
        case TYPE_STRING:
            expr->as.literal.value.s = strdup(value.value.s);
            break;
        default:
            expr->as.literal.value.ptr = value.value.ptr;
            break;
    }
    
    return expr;
}

Expr* ast_create_variable_expr(char* name) {
    Expr* expr = prism_alloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = strdup(name);
    return expr;
}

Expr* ast_create_call_expr(Expr* callee, Expr** args, int arg_count) {
    Expr* expr = prism_alloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->as.call.callee = callee;
    expr->as.call.args = prism_alloc(sizeof(Expr*) * arg_count);
    expr->as.call.arg_count = arg_count;
    
    // Copy args
    for (int i = 0; i < arg_count; i++) {
        expr->as.call.args[i] = args[i];
    }
    
    return expr;
}

Expr* ast_create_binary_expr(char* op, Expr* left, Expr* right) {
    Expr* expr = prism_alloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.op = strdup(op);
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}

Expr* ast_create_unary_expr(char* op, Expr* operand) {
    Expr* expr = prism_alloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.op = strdup(op);
    expr->as.unary.operand = operand;
    return expr;
}

Stmt* ast_create_expr_stmt(Expr* expr) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_EXPR;
    stmt->as.expr = expr;
    return stmt;
}

Stmt* ast_create_var_decl_stmt(char* name, PrismType type, bool exposed, bool internal, Expr* initializer) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_VAR_DECL;
    stmt->as.var_decl.name = strdup(name);
    stmt->as.var_decl.type = type;
    stmt->as.var_decl.exposed = exposed;
    stmt->as.var_decl.internal = internal;
    stmt->as.var_decl.initializer = initializer;
    return stmt;
}

Stmt* ast_create_func_decl_stmt(char* name, char** params, PrismType* param_types, int param_count, 
                               Stmt** body, int body_count, PrismType return_type) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_FUNC_DECL;
    stmt->as.func_decl.name = strdup(name);
    
    // Copy parameters
    stmt->as.func_decl.params = prism_alloc(sizeof(char*) * param_count);
    stmt->as.func_decl.param_types = prism_alloc(sizeof(PrismType) * param_count);
    stmt->as.func_decl.param_count = param_count;
    
    for (int i = 0; i < param_count; i++) {
        stmt->as.func_decl.params[i] = strdup(params[i]);
        stmt->as.func_decl.param_types[i] = param_types[i];
    }
    
    // Copy body
    stmt->as.func_decl.body = prism_alloc(sizeof(Stmt*) * body_count);
    stmt->as.func_decl.body_count = body_count;
    
    for (int i = 0; i < body_count; i++) {
        stmt->as.func_decl.body[i] = body[i];
    }
    
    stmt->as.func_decl.return_type = return_type;
    
    return stmt;
}

Stmt* ast_create_prism_decl_stmt(char* name, Stmt** body, int body_count, PrismType return_type) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_PRISM_DECL;
    stmt->as.prism_decl.name = strdup(name);
    
    // Copy body
    stmt->as.prism_decl.body = prism_alloc(sizeof(Stmt*) * body_count);
    stmt->as.prism_decl.body_count = body_count;
    
    for (int i = 0; i < body_count; i++) {
        stmt->as.prism_decl.body[i] = body[i];
    }
    
    stmt->as.prism_decl.return_type = return_type;
    
    return stmt;
}

Stmt* ast_create_return_stmt(Expr* value) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.return_stmt.value = value;
    return stmt;
}

Stmt* ast_create_call_stmt(Expr* callee, Expr** args, int arg_count) {
    Stmt* stmt = prism_alloc(sizeof(Stmt));
    stmt->type = STMT_CALL;
    stmt->as.call.callee = callee;
    stmt->as.call.args = prism_alloc(sizeof(Expr*) * arg_count);
    stmt->as.call.arg_count = arg_count;
    
    // Copy args
    for (int i = 0; i < arg_count; i++) {
        stmt->as.call.args[i] = args[i];
    }
    
    return stmt;
}

void ast_free_expr(Expr* expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case EXPR_LITERAL:
            if (expr->as.literal.type == TYPE_STRING && expr->as.literal.value.s) {
                prism_free(expr->as.literal.value.s);
            }
            break;
        case EXPR_VARIABLE:
            prism_free(expr->as.variable.name);
            break;
        case EXPR_CALL:
            ast_free_expr(expr->as.call.callee);
            for (int i = 0; i < expr->as.call.arg_count; i++) {
                ast_free_expr(expr->as.call.args[i]);
            }
            prism_free(expr->as.call.args);
            break;
        case EXPR_BINARY:
            prism_free(expr->as.binary.op);
            ast_free_expr(expr->as.binary.left);
            ast_free_expr(expr->as.binary.right);
            break;
        case EXPR_UNARY:
            prism_free(expr->as.unary.op);
            ast_free_expr(expr->as.unary.operand);
            break;
    }
    
    prism_free(expr);
}

void ast_free_stmt(Stmt* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case STMT_EXPR:
            ast_free_expr(stmt->as.expr);
            break;
        case STMT_VAR_DECL:
            prism_free(stmt->as.var_decl.name);
            ast_free_expr(stmt->as.var_decl.initializer);
            break;
        case STMT_FUNC_DECL:
            prism_free(stmt->as.func_decl.name);
            for (int i = 0; i < stmt->as.func_decl.param_count; i++) {
                prism_free(stmt->as.func_decl.params[i]);
            }
            prism_free(stmt->as.func_decl.params);
            prism_free(stmt->as.func_decl.param_types);
            for (int i = 0; i < stmt->as.func_decl.body_count; i++) {
                ast_free_stmt(stmt->as.func_decl.body[i]);
            }
            prism_free(stmt->as.func_decl.body);
            break;
        case STMT_PRISM_DECL:
            prism_free(stmt->as.prism_decl.name);
            for (int i = 0; i < stmt->as.prism_decl.body_count; i++) {
                ast_free_stmt(stmt->as.prism_decl.body[i]);
            }
            prism_free(stmt->as.prism_decl.body);
            break;
        case STMT_RETURN:
            ast_free_expr(stmt->as.return_stmt.value);
            break;
        case STMT_CALL:
            ast_free_expr(stmt->as.call.callee);
            for (int i = 0; i < stmt->as.call.arg_count; i++) {
                ast_free_expr(stmt->as.call.args[i]);
            }
            prism_free(stmt->as.call.args);
            break;
    }
    
    prism_free(stmt);
}