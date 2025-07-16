#ifndef PROCESS_H
#define PROCESS_H

#include "memory.h"

// Process states
#define PROCESS_READY    0
#define PROCESS_RUNNING  1
#define PROCESS_BLOCKED  2
#define PROCESS_TERMINATED 3

// Process structure
typedef struct process {
    unsigned int pid;
    unsigned int state;
    void* stack;
    unsigned int stack_size;
    void (*entry_point)(void);
    struct process* next;
} process_t;

// Process management functions
void process_init(void);
process_t* process_create(void (*entry_point)(void), unsigned int stack_size);
void process_start(process_t* process);
void process_yield(void);
void process_exit(void);
process_t* process_get_current(void);
void process_schedule(void);

// Process management state
extern process_t* current_process;
extern process_t* process_list;
extern unsigned int next_pid;

// Maximum number of processes
#define MAX_PROCESSES 16

#endif 