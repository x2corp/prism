#ifndef PRISM_UTIL_H
#define PRISM_UTIL_H

#include <stdio.h>
#include "types.h"

char* prism_read_file(const char* path);
void prism_write_file(const char* path, const char* content);
char* prism_format_value(PrismValue value);
void prism_print_value(PrismValue value);

#endif /* PRISM_UTIL_H */