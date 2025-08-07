#ifndef USER_H
#define USER_H

#include "memory.h"
#include "process.h"

// Define our own integer types for bare-metal environment
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

// User space constants
#define USER_STACK_SIZE 4096
#define USER_HEAP_SIZE 8192
#define MAX_USER_PROGRAMS 16
#define MAX_PROGRAM_SIZE 16384

// User program structure
typedef struct user_program {
    char name[32];
    void* code;
    uint32_t size;
    uint32_t entry_point;
    int used;
} user_program_t;

// System call numbers
#define SYS_EXIT    0
#define SYS_WRITE   1
#define SYS_READ    2
#define SYS_OPEN    3
#define SYS_CLOSE   4
#define SYS_MALLOC  5
#define SYS_FREE    6

// User space functions
void user_init(void);
int user_load_program(const char* name, const void* code, uint32_t size);
int user_run_program(const char* name);
void user_switch_to_kernel(void);
void user_switch_to_user(void* entry_point, void* stack);

// System call handler
uint32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// User program management
user_program_t* user_find_program(const char* name);
void user_list_programs(void);
int user_remove_program(const char* name);
void user_load_builtin_programs(void);
void user_create_demo_programs(void);
int user_compile_and_load(const char* name, const char* source_code);
int user_load_from_file(const char* filename);
int user_load_binary_from_system(const char* program_name);

#endif