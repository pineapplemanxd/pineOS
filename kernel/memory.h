#ifndef MEMORY_H
#define MEMORY_H

// Memory management functions
void memory_init(void);
void* memory_alloc(unsigned int size);
void memory_free(void* ptr);
void memory_copy(void* dest, const void* src, unsigned int size);
void memory_set(void* dest, unsigned char value, unsigned int size);
unsigned int memory_get_free(void);

// Memory layout constants
#define MEMORY_START 0x10000
#define MEMORY_END   0x100000
#define PAGE_SIZE    4096
#define MAX_PAGES    64

// Memory block structure
typedef struct memory_block {
    unsigned int size;
    int used;
    struct memory_block* next;
} memory_block_t;

// Memory management state
extern memory_block_t* memory_head;
extern unsigned int memory_total;
extern unsigned int memory_used;

#endif 