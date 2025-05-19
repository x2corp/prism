#ifndef PRISM_LEXER_H
#define PRISM_LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_ARROW,
    TOKEN_RETURN_TYPE,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_FUNCTION,
    TOKEN_PRISM,
    TOKEN_INTERNAL,
    TOKEN_EXPOSED,
    TOKEN_DOT,
    TOKEN_RUN,
    TOKEN_NONE
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

typedef struct {
    const char* source;
    const char* filename;
    int start;
    int current;
    int line;
    int column;
    Token* tokens;
    int token_count;
    int token_capacity;
} Lexer;

Lexer* lexer_create(const char* source, const char* filename);
void lexer_free(Lexer* lexer);
void lexer_scan_tokens(Lexer* lexer);
Token* lexer_get_tokens(Lexer* lexer, int* count);

#endif /* PRISM_LEXER_H */