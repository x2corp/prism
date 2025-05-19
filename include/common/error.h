#ifndef PRISM_ERROR_H
#define PRISM_ERROR_H

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_NAME,
    ERROR_MEMORY,
    ERROR_RUNTIME
} PrismErrorType;

typedef struct {
    PrismErrorType type;
    char* message;
    int line;
    int column;
    char* filename;
} PrismError;

void prism_error(const char* format, ...);
void prism_error_at(const char* filename, int line, int column, const char* format, ...);
PrismError* prism_get_last_error();
void prism_clear_error();

#endif /* PRISM_ERROR_H */