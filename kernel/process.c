#include "process.h"

// Global variables
process_t* current_process = 0;
process_t* process_list = 0;
unsigned int next_pid = 1;

// Process initialization
void process_init(void) {
    current_process = 0;
    process_list = 0;
    next_pid = 1;
}

// Create a new process
process_t* process_create(void (*entry_point)(void), unsigned int stack_size) {
    // Allocate process structure
    process_t* process = (process_t*)memory_alloc(sizeof(process_t));
    if (!process) return 0;
    
    // Allocate stack
    void* stack = memory_alloc(stack_size);
    if (!stack) {
        memory_free(process);
        return 0;
    }
    
    // Initialize process
    process->pid = next_pid++;
    process->state = PROCESS_READY;
    process->stack = stack;
    process->stack_size = stack_size;
    process->entry_point = entry_point;
    process->next = 0;
    
    // Add to process list
    if (!process_list) {
        process_list = process;
    } else {
        process_t* current = process_list;
        while (current->next) {
            current = current->next;
        }
        current->next = process;
    }
    
    return process;
}

// Start a process
void process_start(process_t* process) {
    if (!process || process->state != PROCESS_READY) return;
    
    process->state = PROCESS_RUNNING;
    current_process = process;
    
    // In a real OS, this would set up the process context
    // and jump to the entry point. For simplicity, we'll
    // just call the entry point directly.
    if (process->entry_point) {
        process->entry_point();
    }
}

// Yield control to another process
void process_yield(void) {
    if (!current_process) return;
    
    // Find next ready process
    process_t* next = current_process->next;
    while (next && next->state != PROCESS_READY) {
        next = next->next;
    }
    
    if (!next) {
        // Look from beginning of list
        next = process_list;
        while (next && next->state != PROCESS_READY) {
            next = next->next;
        }
    }
    
    if (next && next != current_process) {
        current_process->state = PROCESS_READY;
        next->state = PROCESS_RUNNING;
        current_process = next;
        
        // In a real OS, this would perform context switch
        // For now, we'll just return and let the scheduler
        // handle it in the main loop
    }
}

// Exit current process
void process_exit(void) {
    if (!current_process) return;
    
    current_process->state = PROCESS_TERMINATED;
    
    // Free process resources
    if (current_process->stack) {
        memory_free(current_process->stack);
        current_process->stack = 0;
    }
    
    // Remove from process list
    if (process_list == current_process) {
        process_list = current_process->next;
    } else {
        process_t* prev = process_list;
        while (prev && prev->next != current_process) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = current_process->next;
        }
    }
    
    // Free process structure
    memory_free(current_process);
    current_process = 0;
    
    // Schedule next process
    process_schedule();
}

// Get current process
process_t* process_get_current(void) {
    return current_process;
}

// Simple round-robin scheduler
void process_schedule(void) {
    if (!current_process || current_process->state != PROCESS_RUNNING) {
        // Find a ready process to run
        process_t* next = process_list;
        while (next) {
            if (next->state == PROCESS_READY) {
                next->state = PROCESS_RUNNING;
                current_process = next;
                break;
            }
            next = next->next;
        }
    }
}

// Simple test process function
void test_process_function(void) {
    // This is a simple test process that just prints a message
    // In a real OS, this would be a separate program
    // For now, we'll just simulate it
    return;
}

// Create a simple test process
process_t* create_test_process(void) {
    return process_create(test_process_function, 1024);
}

// Get process count
int get_process_count(void) {
    int count = 0;
    process_t* current = process_list;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}

// Get ready process count
int get_ready_process_count(void) {
    int count = 0;
    process_t* current = process_list;
    while (current) {
        if (current->state == PROCESS_READY) {
            count++;
        }
        current = current->next;
    }
    return count;
} 