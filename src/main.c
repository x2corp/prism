#include "../include/core/lexer.h"
#include "../include/core/parser.h"
#include "../include/core/ast.h"
#include "../include/core/symtab.h"
#include "../include/core/vm.h"
#include "../include/common/error.h"
#include "../include/common/memory.h"
#include "../include/common/util.h"
#include "../include/lib/std.h"
#include "../include/lib/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [options] [script]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help        Show this help message\n");
    printf("  -v, --version     Show version information\n");
    printf("  -i, --interactive Run in interactive mode\n");
    printf("  -c, --compile     Compile script to bytecode\n");
}

static void print_version() {
    printf("Prism v0.1-beta\n");
    printf("Copyright (c) 2025 x2corp\n");
}

static void repl() {
    VM* vm = vm_create();
    char line[1024];
    printf("Prism v0.1-beta\n");
    printf("Type 'exit' to quit\n");
    
    for (;;) {
        printf("> ");
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        
        if (strcmp(line, "exit\n") == 0) {
            break;
        }
        
        vm_interpret(vm, line, "repl");
    }
    
    vm_free(vm);
}

static void run_file(const char* path) {
    char* source = prism_read_file(path);
    if (!source) {
        fprintf(stderr, "Could not read file '%s'\n", path);
        exit(74);
    }
    
    VM* vm = vm_create();
    InterpretResult result = vm_interpret(vm, source, path);
    vm_free(vm);
    prism_free(source);
    
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char* argv[]) {
    // Initialize standard library and IO
    prism_std_init();
    prism_io_init();
    
    if (argc == 1) {
        // No arguments, run REPL
        repl();
    } else if (argc == 2) {
        // Single argument
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_usage(argv[0]);
        } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            print_version();
        } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
            repl();
        } else {
            // Assume it's a script file
            run_file(argv[1]);
        }
    } else {
        // Multiple arguments, process them
        bool interactive = false;
        bool compile = false;
        const char* script_file = NULL;
        
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage(argv[0]);
                return 0;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                print_version();
                return 0;
            } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
                interactive = true;
            } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compile") == 0) {
                compile = true;
            } else if (argv[i][0] != '-') {
                script_file = argv[i];
            }
        }
        
        if (compile && script_file) {
            fprintf(stderr, "Compilation to bytecode not implemented yet\n");
            return 1;
        } else if (script_file) {
            run_file(script_file);
            if (interactive) {
                repl();
            }
        } else if (interactive) {
            repl();
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Cleanup
    prism_std_cleanup();
    prism_io_cleanup();
    
    return 0;
}