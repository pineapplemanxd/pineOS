#include "memory.h"

// Global variables
memory_block_t* memory_head = 0;
unsigned int memory_total = 0;
unsigned int memory_used = 0;

// Memory initialization
void memory_init(void) {
    // Initialize memory management
    memory_total = MEMORY_END - MEMORY_START;
    memory_used = 0;
    
    // Create initial free block
    memory_head = (memory_block_t*)MEMORY_START;
    memory_head->size = memory_total - sizeof(memory_block_t);
    memory_head->used = 0;
    memory_head->next = 0;
}

// Memory allocation using first-fit algorithm
void* memory_alloc(unsigned int size) {
    if (size == 0) return 0;
    
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    memory_block_t* current = memory_head;
    memory_block_t* best_fit = 0;
    unsigned int best_size = 0xFFFFFFFF;
    
    // Find best fit
    while (current) {
        if (!current->used && current->size >= size) {
            if (current->size < best_size) {
                best_fit = current;
                best_size = current->size;
            }
        }
        current = current->next;
    }
    
    if (!best_fit) return 0;  // No suitable block found
    
    // Split block if it's much larger than needed
    if (best_fit->size > size + sizeof(memory_block_t) + 16) {
        memory_block_t* new_block = (memory_block_t*)((char*)best_fit + sizeof(memory_block_t) + size);
        new_block->size = best_fit->size - size - sizeof(memory_block_t);
        new_block->used = 0;
        new_block->next = best_fit->next;
        
        best_fit->size = size;
        best_fit->next = new_block;
    }
    
    best_fit->used = 1;
    memory_used += best_fit->size;
    
    return (char*)best_fit + sizeof(memory_block_t);
}

// Memory deallocation
void memory_free(void* ptr) {
    if (!ptr) return;
    
    memory_block_t* block = (memory_block_t*)((char*)ptr - sizeof(memory_block_t));
    
    if (!block->used) return;  // Already freed
    
    block->used = 0;
    memory_used -= block->size;
    
    // Merge with next block if it's also free
    if (block->next && !block->next->used) {
        block->size += block->next->size + sizeof(memory_block_t);
        block->next = block->next->next;
    }
    
    // Merge with previous block if it's also free
    memory_block_t* current = memory_head;
    while (current && current->next != block) {
        current = current->next;
    }
    
    if (current && !current->used) {
        current->size += block->size + sizeof(memory_block_t);
        current->next = block->next;
    }
}

// Memory copy function
void memory_copy(void* dest, const void* src, unsigned int size) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (unsigned int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// Memory set function
void memory_set(void* dest, unsigned char value, unsigned int size) {
    unsigned char* d = (unsigned char*)dest;
    
    for (unsigned int i = 0; i < size; i++) {
        d[i] = value;
    }
}

// Get free memory
unsigned int memory_get_free(void) {
    return memory_total - memory_used;
}

// Simple memory test function
int memory_test(void) {
    // Test basic allocation
    void* ptr1 = memory_alloc(100);
    if (!ptr1) return 0;
    
    void* ptr2 = memory_alloc(200);
    if (!ptr2) {
        memory_free(ptr1);
        return 0;
    }
    
    void* ptr3 = memory_alloc(50);
    if (!ptr3) {
        memory_free(ptr1);
        memory_free(ptr2);
        return 0;
    }
    
    // Test memory write/read
    memory_set(ptr1, 0xAA, 100);
    memory_set(ptr2, 0xBB, 200);
    memory_set(ptr3, 0xCC, 50);
    
    // Verify memory contents
    unsigned char* p1 = (unsigned char*)ptr1;
    unsigned char* p2 = (unsigned char*)ptr2;
    unsigned char* p3 = (unsigned char*)ptr3;
    
    for (int i = 0; i < 100; i++) {
        if (p1[i] != 0xAA) {
            memory_free(ptr1);
            memory_free(ptr2);
            memory_free(ptr3);
            return 0;
        }
    }
    
    for (int i = 0; i < 200; i++) {
        if (p2[i] != 0xBB) {
            memory_free(ptr1);
            memory_free(ptr2);
            memory_free(ptr3);
            return 0;
        }
    }
    
    for (int i = 0; i < 50; i++) {
        if (p3[i] != 0xCC) {
            memory_free(ptr1);
            memory_free(ptr2);
            memory_free(ptr3);
            return 0;
        }
    }
    
    // Test deallocation
    memory_free(ptr2);
    memory_free(ptr1);
    memory_free(ptr3);
    
    return 1;
} 