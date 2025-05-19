#include "../../include/common/error.h"
#include "../../include/common/memory.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static PrismError last_error = {ERROR_NONE, NULL, 0, 0, NULL};

void prism_error(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (last_error.message) prism_free(last_error.message);
    last_error.type = ERROR_RUNTIME;
    last_error.message = strdup(buffer);
    last_error.line = 0;
    last_error.column = 0;
    
    fprintf(stderr, "Error: %s\n", buffer);
}

void prism_error_at(const char* filename, int line, int column, const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (last_error.message) prism_free(last_error.message);
    if (last_error.filename) prism_free(last_error.filename);
    
    last_error.type = ERROR_SYNTAX;
    last_error.message = strdup(buffer);
    last_error.line = line;
    last_error.column = column;
    last_error.filename = strdup(filename);
    
    fprintf(stderr, "%s:%d:%d: Error: %s\n", filename, line, column, buffer);
}

PrismError* prism_get_last_error() {
    return &last_error;
}

void prism_clear_error() {
    if (last_error.message) {
        prism_free(last_error.message);
        last_error.message = NULL;
    }
    if (last_error.filename) {
        prism_free(last_error.filename);
        last_error.filename = NULL;
    }
    last_error.type = ERROR_NONE;
    last_error.line = 0;
    last_error.column = 0;
}