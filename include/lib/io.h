#ifndef PRISM_IO_H
#define PRISM_IO_H

#include "../common/types.h"

void prism_io_init();
void prism_io_cleanup();

// Core IO library functions
PrismValue prism_io_read_file(PrismValue* args, int arg_count);
PrismValue prism_io_write_file(PrismValue* args, int arg_count);
PrismValue prism_io_append_file(PrismValue* args, int arg_count);
PrismValue prism_io_file_exists(PrismValue* args, int arg_count);
PrismValue prism_io_delete_file(PrismValue* args, int arg_count);

// Register all IO library functions in the VM
void prism_io_register_all(void* vm);

#endif /* PRISM_IO_H */