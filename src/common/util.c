#include "../../include/common/util.h"
#include "../../include/common/memory.h"
#include "../../include/common/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* prism_read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        prism_error("Could not open file '%s'", path);
        return NULL;
    }
    
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    
    char* buffer = prism_alloc(file_size + 1);
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        prism_free(buffer);
        prism_error("Could not read file '%s'", path);
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

void prism_write_file(const char* path, const char* content) {
    FILE* file = fopen(path, "wb");
    if (!file) {
        prism_error("Could not open file '%s' for writing", path);
        return;
    }
    
    fputs(content, file);
    fclose(file);
}

char* prism_format_value(PrismValue value) {
    char buffer[128];
    
    switch (value.type) {
        case TYPE_INT:
            snprintf(buffer, sizeof(buffer), "%lld", (long long)value.value.i);
            break;
        case TYPE_FLOAT:
            snprintf(buffer, sizeof(buffer), "%g", value.value.f);
            break;
        case TYPE_BOOL:
            snprintf(buffer, sizeof(buffer), "%s", value.value.b ? "true" : "false");
            break;
        case TYPE_STRING:
            return strdup(value.value.s ? value.value.s : "");
        case TYPE_NONE:
            snprintf(buffer, sizeof(buffer), "None");
            break;
        default:
            snprintf(buffer, sizeof(buffer), "[%s]", prism_type_to_string(value.type));
            break;
    }
    
    return strdup(buffer);
}

void prism_print_value(PrismValue value) {
    char* formatted = prism_format_value(value);
    printf("%s", formatted);
    prism_free(formatted);
}