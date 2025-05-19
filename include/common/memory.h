#ifndef PRISM_MEMORY_H
#define PRISM_MEMORY_H

#include <stdlib.h>
#include "../common/types.h"

/* Memory management functions */
void* prism_alloc(size_t size);
void* prism_realloc(void* ptr, size_t size);
void prism_free(void* ptr);

/* Value management */
PrismValue* prism_value_create(PrismType type);
void prism_value_free(PrismValue* value);

/* Variable management */
PrismVariable* prism_var_create(char* name, PrismType type, bool exposed, bool internal);
void prism_var_free(PrismVariable* var);

#endif /* PRISM_MEMORY_H */