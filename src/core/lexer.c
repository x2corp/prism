#include "../../include/core/lexer.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_TOKEN_CAPACITY 64

static bool is_alpha(char c) {
    return isalpha(c) || c == '_';
}

static bool is_alphanumeric(char c) {
    return isalnum(c) || c == '_';
}

Lexer* lexer_create(const char* source, const char* filename) {
    Lexer* lexer = prism_alloc(sizeof(Lexer));
    lexer->source = source;
    lexer->filename = filename;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 0;
    lexer->token_count = 0;
    lexer->token_capacity = INITIAL_TOKEN_CAPACITY;
    lexer->tokens = prism_alloc(sizeof(Token) * lexer->token_capacity);
    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (!lexer) return;
    
    for (int i = 0; i < lexer->token_count; i++) {
        if (lexer->tokens[i].lexeme) prism_free(lexer->tokens[i].lexeme);
    }
    
    prism_free(lexer->tokens);
    prism_free(lexer);
}

static void add_token(Lexer* lexer, TokenType type) {
    if (lexer->token_count >= lexer->token_capacity) {
        lexer->token_capacity *= 2;
        lexer->tokens = prism_realloc(lexer->tokens, sizeof(Token) * lexer->token_capacity);
    }
    
    int length = lexer->current - lexer->start;
    char* lexeme = prism_alloc(length + 1);
    strncpy(lexeme, lexer->source + lexer->start, length);
    lexeme[length] = '\0';
    
    lexer->tokens[lexer->token_count].type = type;
    lexer->tokens[lexer->token_count].lexeme = lexeme;
    lexer->tokens[lexer->token_count].line = lexer->line;
    lexer->tokens[lexer->token_count].column = lexer->column - length;
    
    lexer->token_count++;
}

static bool is_at_end(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
}

static char advance(Lexer* lexer) {
    lexer->current++;
    lexer->column++;
    return lexer->source[lexer->current - 1];
}

static char peek(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer) || lexer->source[lexer->current + 1] == '\0') return '\0';
    return lexer->source[lexer->current + 1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    
    lexer->current++;
    lexer->column++;
    return true;
}

static void skip_whitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 0;
                advance(lexer);
                break;
            case '!':
                if (peek_next(lexer) == '!') {
                    // Comment until end of line
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static void scan_identifier(Lexer* lexer) {
    while (is_alphanumeric(peek(lexer))) {
        advance(lexer);
    }
    
    // Check for keywords
    int length = lexer->current - lexer->start;
    char* text = prism_alloc(length + 1);
    strncpy(text, lexer->source + lexer->start, length);
    text[length] = '\0';
    
    TokenType type = TOKEN_IDENTIFIER;
    
    if (strcmp(text, "function") == 0) type = TOKEN_FUNCTION;
    else if (strcmp(text, "prism") == 0) type = TOKEN_PRISM;
    else if (strcmp(text, "internal") == 0) type = TOKEN_INTERNAL;
    else if (strcmp(text, "exposed") == 0) type = TOKEN_EXPOSED;
    else if (strcmp(text, "run") == 0) type = TOKEN_RUN;
    else if (strcmp(text, "None") == 0) type = TOKEN_NONE;
    
    prism_free(text);
    add_token(lexer, type);
}

static void scan_number(Lexer* lexer) {
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }
    
    // Look for decimal
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        // Consume the .
        advance(lexer);
        
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        
        add_token(lexer, TOKEN_FLOAT);
    } else {
        add_token(lexer, TOKEN_INTEGER);
    }
}

static void scan_string(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 0;
        }
        advance(lexer);
    }
    
    if (is_at_end(lexer)) {
        prism_error_at(lexer->filename, lexer->line, lexer->column, "Unterminated string");
        return;
    }
    
    // Consume closing "
    advance(lexer);
    
    // Trim surrounding quotes
    int length = lexer->current - lexer->start - 2;
    char* value = prism_alloc(length + 1);
    strncpy(value, lexer->source + lexer->start + 1, length);
    value[length] = '\0';
    
    add_token(lexer, TOKEN_STRING);
    prism_free(value);
}

static void scan_token(Lexer* lexer) {
    char c = advance(lexer);
    
    switch (c) {
        case '(': add_token(lexer, TOKEN_LPAREN); break;
        case ')': add_token(lexer, TOKEN_RPAREN); break;
        case '[': add_token(lexer, TOKEN_LBRACKET); break;
        case ']': add_token(lexer, TOKEN_RBRACKET); break;
        case ':': add_token(lexer, TOKEN_COLON); break;
        case ',': add_token(lexer, TOKEN_COMMA); break;
        case '.': add_token(lexer, TOKEN_DOT); break;
        
        case '-':
            if (match(lexer, '>')) {
                add_token(lexer, TOKEN_ARROW);
            } else {
                prism_error_at(lexer->filename, lexer->line, lexer->column, "Unexpected character");
            }
            break;
            
        case '>':
            if (match(lexer, '>')) {
                add_token(lexer, TOKEN_RETURN_TYPE);
            } else {
                prism_error_at(lexer->filename, lexer->line, lexer->column, "Unexpected character");
            }
            break;
            
        case '"':
            scan_string(lexer);
            break;
            
        default:
            if (isdigit(c)) {
                scan_number(lexer);
            } else if (is_alpha(c)) {
                scan_identifier(lexer);
            } else {
                prism_error_at(lexer->filename, lexer->line, lexer->column, "Unexpected character");
            }
            break;
    }
}

void lexer_scan_tokens(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        lexer->start = lexer->current;
        skip_whitespace(lexer);
        if (!is_at_end(lexer)) {
            scan_token(lexer);
        }
    }
    
    // Add EOF token
    lexer->tokens[lexer->token_count].type = TOKEN_EOF;
    lexer->tokens[lexer->token_count].lexeme = NULL;
    lexer->tokens[lexer->token_count].line = lexer->line;
    lexer->tokens[lexer->token_count].column = lexer->column;
    lexer->token_count++;
}

Token* lexer_get_tokens(Lexer* lexer, int* count) {
    *count = lexer->token_count;
    return lexer->tokens;
}