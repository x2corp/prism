#ifndef PRISM_STD_H
#define PRISM_STD_H

#include "../common/types.h"

void prism_std_init();
void prism_std_cleanup();

// Core standard library functions
PrismValue prism_std_print(PrismValue* args, int arg_count);
PrismValue prism_std_render(PrismValue* args, int arg_count);
PrismValue prism_std_input(PrismValue* args, int arg_count);
PrismValue prism_std_type(PrismValue* args, int arg_count);
PrismValue prism_std_string(PrismValue* args, int arg_count);
PrismValue prism_std_int(PrismValue* args, int arg_count);
PrismValue prism_std_float(PrismValue* args, int arg_count);
PrismValue prism_std_bool(PrismValue* args, int arg_count);

// Register all standard library functions in the VM
void prism_std_register_all(void* vm);

#endif /* PRISM_STD_H */