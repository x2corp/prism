#ifndef PRISM_PARSER_H
#define PRISM_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Token* tokens;
    int token_count;
    int current;
} Parser;

Parser* parser_create(Token* tokens, int token_count);
void parser_free(Parser* parser);

Program* parser_parse(Parser* parser);

#endif /* PRISM_PARSER_H */