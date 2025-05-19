#ifndef PRISM_AST_H
#define PRISM_AST_H

#include "../common/types.h"

typedef enum {
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_CALL,
    EXPR_BINARY,
    EXPR_UNARY
} ExprType;

typedef enum {
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_FUNC_DECL,
    STMT_PRISM_DECL,
    STMT_RETURN,
    STMT_CALL
} StmtType;

typedef struct Expr {
    ExprType type;
    union {
        PrismValue literal;
        struct {
            char* name;
        } variable;
        struct {
            struct Expr* callee;
            struct Expr** args;
            int arg_count;
        } call;
        struct {
            char* op;
            struct Expr* left;
            struct Expr* right;
        } binary;
        struct {
            char* op;
            struct Expr* operand;
        } unary;
    } as;
} Expr;

typedef struct {
    char* name;
    PrismType type;
    bool exposed;
    bool internal;
    Expr* initializer;
} VarDecl;

typedef struct {
    char* name;
    char** params;
    PrismType* param_types;
    int param_count;
    struct Stmt** body;
    int body_count;
    PrismType return_type;
} FuncDecl;

typedef struct {
    char* name;
    struct Stmt** body;
    int body_count;
    PrismType return_type;
} PrismDecl;

typedef struct Stmt {
    StmtType type;
    union {
        Expr* expr;
        VarDecl var_decl;
        FuncDecl func_decl;
        PrismDecl prism_decl;
        struct {
            Expr* value;
        } return_stmt;
        struct {
            Expr* callee;
            Expr** args;
            int arg_count;
        } call;
    } as;
} Stmt;

typedef struct {
    Stmt** statements;
    int count;
    int capacity;
} Program;

Program* ast_create_program();
void ast_free_program(Program* program);
void ast_add_statement(Program* program, Stmt* stmt);

Expr* ast_create_literal_expr(PrismValue value);
Expr* ast_create_variable_expr(char* name);
Expr* ast_create_call_expr(Expr* callee, Expr** args, int arg_count);
Expr* ast_create_binary_expr(char* op, Expr* left, Expr* right);
Expr* ast_create_unary_expr(char* op, Expr* operand);

Stmt* ast_create_expr_stmt(Expr* expr);
Stmt* ast_create_var_decl_stmt(char* name, PrismType type, bool exposed, bool internal, Expr* initializer);
Stmt* ast_create_func_decl_stmt(char* name, char** params, PrismType* param_types, int param_count, Stmt** body, int body_count, PrismType return_type);
Stmt* ast_create_prism_decl_stmt(char* name, Stmt** body, int body_count, PrismType return_type);
Stmt* ast_create_return_stmt(Expr* value);
Stmt* ast_create_call_stmt(Expr* callee, Expr** args, int arg_count);

void ast_free_expr(Expr* expr);
void ast_free_stmt(Stmt* stmt);

#endif /* PRISM_AST_H */