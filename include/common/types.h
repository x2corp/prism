#ifndef PRISM_TYPES_H
#define PRISM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TYPE_NONE,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_FUNCTION,
    TYPE_NATIVE,
    TYPE_PRISM
} PrismType;

typedef struct {
    PrismType type;
    union {
        int64_t i;
        double f;
        bool b;
        char* s;
        void* ptr;
    } value;
} PrismValue;

typedef struct {
    char* name;
    PrismType type;
    bool exposed;
    bool internal;
    PrismValue value;
} PrismVariable;

const char* prism_type_to_string(PrismType type);
PrismValue prism_value_convert(PrismValue value, PrismType target_type);

#endif /* PRISM_TYPES_H */