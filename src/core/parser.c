#include "../../include/core/parser.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <string.h>

Parser* parser_create(Token* tokens, int token_count) {
    Parser* parser = prism_alloc(sizeof(Parser));
    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->current = 0;
    return parser;
}

void parser_free(Parser* parser) {
    if (parser) prism_free(parser);
}

static Token* peek(Parser* parser) {
    return &parser->tokens[parser->current];
}

static Token* previous(Parser* parser) {
    return &parser->tokens[parser->current - 1];
}

static bool is_at_end(Parser* parser) {
    return peek(parser)->type == TOKEN_EOF;
}

static Token* advance(Parser* parser) {
    if (!is_at_end(parser)) parser->current++;
    return previous(parser);
}

static bool check(Parser* parser, TokenType type) {
    if (is_at_end(parser)) return false;
    return peek(parser)->type == type;
}

static bool match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) {
        advance(parser);
        return;
    }
    
    Token* token = peek(parser);
    prism_error_at(token->lexeme, token->line, token->column, "%s", message);
}

static PrismType parse_type(Parser* parser) {
    Token* token = advance(parser);
    
    if (strcmp(token->lexeme, "int") == 0) return TYPE_INT;
    if (strcmp(token->lexeme, "float") == 0) return TYPE_FLOAT;
    if (strcmp(token->lexeme, "bool") == 0) return TYPE_BOOL;
    if (strcmp(token->lexeme, "string") == 0) return TYPE_STRING;
    if (strcmp(token->lexeme, "None") == 0) return TYPE_NONE;
    
    prism_error_at(token->lexeme, token->line, token->column, "Unknown type");
    return TYPE_NONE;
}

static Expr* parse_expression(Parser* parser);

static Expr* parse_primary(Parser* parser) {
    if (match(parser, TOKEN_INTEGER)) {
        PrismValue value;
        value.type = TYPE_INT;
        value.value.i = atoi(previous(parser)->lexeme);
        return ast_create_literal_expr(value);
    }
    
    if (match(parser, TOKEN_FLOAT)) {
        PrismValue value;
        value.type = TYPE_FLOAT;
        value.value.f = atof(previous(parser)->lexeme);
        return ast_create_literal_expr(value);
    }
    
    if (match(parser, TOKEN_STRING)) {
        PrismValue value;
        value.type = TYPE_STRING;
        value.value.s = strdup(previous(parser)->lexeme);
        return ast_create_literal_expr(value);
    }
    
    if (match(parser, TOKEN_IDENTIFIER)) {
        return ast_create_variable_expr(previous(parser)->lexeme);
    }
    
    if (match(parser, TOKEN_LPAREN)) {
        Expr* expr = parse_expression(parser);
        consume(parser, TOKEN_RPAREN, "Expect ')' after expression");
        return expr;
    }
    
    Token* token = peek(parser);
    prism_error_at(token->lexeme, token->line, token->column, "Expect expression");
    return NULL;
}

static Expr* parse_call(Parser* parser) {
    Expr* expr = parse_primary(parser);
    
    while (true) {
        if (match(parser, TOKEN_LPAREN)) {
            // Parse function call
            Expr** args = NULL;
            int arg_count = 0;
            
            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    Expr* arg = parse_expression(parser);
                    
                    // Add arg to args array
                    arg_count++;
                    args = prism_realloc(args, sizeof(Expr*) * arg_count);
                    args[arg_count - 1] = arg;
                } while (match(parser, TOKEN_COMMA));
            }
            
            consume(parser, TOKEN_RPAREN, "Expect ')' after arguments");
            expr = ast_create_call_expr(expr, args, arg_count);
            
            if (args) prism_free(args);
        } else if (match(parser, TOKEN_DOT)) {
            // Parse property access
            Token* name = consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'");
            expr = ast_create_variable_expr(name->lexeme);
        } else {
            break;
        }
    }
    
    return expr;
}

static Expr* parse_unary(Parser* parser) {
    // Not implemented yet - placeholder for unary operations
    return parse_call(parser);
}

static Expr* parse_multiplicative(Parser* parser) {
    Expr* expr = parse_unary(parser);
    
    // Not implemented yet - placeholder for multiplication/division operations
    
    return expr;
}

static Expr* parse_additive(Parser* parser) {
    Expr* expr = parse_multiplicative(parser);
    
    // Not implemented yet - placeholder for addition/subtraction operations
    
    return expr;
}

static Expr* parse_expression(Parser* parser) {
    return parse_additive(parser);
}

static Stmt* parse_statement(Parser* parser);

static Stmt* parse_var_declaration(Parser* parser) {
    bool exposed = match(parser, TOKEN_EXPOSED);
    bool internal = match(parser, TOKEN_INTERNAL);
    
    Token* name = consume(parser, TOKEN_IDENTIFIER, "Expect variable name");
    
    consume(parser, TOKEN_ARROW, "Expect '->' after variable name");
    
    Expr* initializer = parse_expression(parser);
    
    return ast_create_var_decl_stmt(name->lexeme, TYPE_NONE, exposed, internal, initializer);
}

static Stmt* parse_function_declaration(Parser* parser) {
    Token* name = consume(parser, TOKEN_IDENTIFIER, "Expect function name");
    
    // Parse parameters
    consume(parser, TOKEN_LBRACKET, "Expect '[' after function name");
    
    char** params = NULL;
    PrismType* param_types = NULL;
    int param_count = 0;
    
    if (!check(parser, TOKEN_RBRACKET)) {
        do {
            Token* param_name = consume(parser, TOKEN_IDENTIFIER, "Expect parameter name");
            consume(parser, TOKEN_COLON, "Expect ':' after parameter name");
            PrismType param_type = parse_type(parser);
            
            // Add parameter to params array
            param_count++;
            params = prism_realloc(params, sizeof(char*) * param_count);
            param_types = prism_realloc(param_types, sizeof(PrismType) * param_count);
            
            params[param_count - 1] = strdup(param_name->lexeme);
            param_types[param_count - 1] = param_type;
        } while (match(parser, TOKEN_COMMA));
    }
    
    consume(parser, TOKEN_RBRACKET, "Expect ']' after parameters");
    
    // Parse function body
    consume(parser, TOKEN_LPAREN, "Expect '(' before function body");
    
    Stmt** body = NULL;
    int body_count = 0;
    
    while (!check(parser, TOKEN_RPAREN) && !is_at_end(parser)) {
        Stmt* stmt = parse_statement(parser);
        
        // Add statement to body array
        body_count++;
        body = prism_realloc(body, sizeof(Stmt*) * body_count);
        body[body_count - 1] = stmt;
    }
    
    consume(parser, TOKEN_RPAREN, "Expect ')' after function body");
    
    // Parse return type
    consume(parser, TOKEN_RETURN_TYPE, "Expect '>>' after function body");
    PrismType return_type = parse_type(parser);
    
    return ast_create_func_decl_stmt(name->lexeme, params, param_types, param_count, body, body_count, return_type);
}

static Stmt* parse_prism_declaration(Parser* parser) {
    Token* name = consume(parser, TOKEN_IDENTIFIER, "Expect prism name");
    
    // Parse prism body
    consume(parser, TOKEN_LPAREN, "Expect '(' before prism body");
    
    Stmt** body = NULL;
    int body_count = 0;
    
    while (!check(parser, TOKEN_RPAREN) && !is_at_end(parser)) {
        Stmt* stmt = parse_statement(parser);
        
        // Add statement to body array
        body_count++;
        body = prism_realloc(body, sizeof(Stmt*) * body_count);
        body[body_count - 1] = stmt;
    }
    
    consume(parser, TOKEN_RPAREN, "Expect ')' after prism body");
    
    // Parse return type
    consume(parser, TOKEN_RETURN_TYPE, "Expect '>>' after prism body");
    PrismType return_type = parse_type(parser);
    
    return ast_create_prism_decl_stmt(name->lexeme, body, body_count, return_type);
}

static Stmt* parse_statement(Parser* parser) {
    if (match(parser, TOKEN_FUNCTION)) {
        return parse_function_declaration(parser);
    }
    
    if (match(parser, TOKEN_PRISM)) {
        return parse_prism_declaration(parser);
    }
    
    if (match(parser, TOKEN_INTERNAL) || match(parser, TOKEN_EXPOSED)) {
        parser->current--; // Backtrack
        return parse_var_declaration(parser);
    }
    
    // Expression statement
    Expr* expr = parse_expression(parser);
    return ast_create_expr_stmt(expr);
}

Program* parser_parse(Parser* parser) {
    Program* program = ast_create_program();
    
    while (!is_at_end(parser)) {
        Stmt* stmt = parse_statement(parser);
        ast_add_statement(program, stmt);
    }
    
    return program;
}