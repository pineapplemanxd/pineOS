#include "userlib.h"

int main(void) {
    puts("User Program Test");
    puts("=================");
    
    puts("Testing string functions...");
    
    char str1[] = "Hello";
    char str2[] = "World";
    char result[64];
    
    strcpy(result, str1);
    puts(result);
    
    if (strcmp(str1, "Hello") == 0) {
        puts("String comparison works!");
    }
    
    puts("Testing memory allocation...");
    void* ptr = sys_malloc(100);
    if (ptr) {
        puts("Memory allocation successful");
        sys_free(ptr);
        puts("Memory freed");
    } else {
        puts("Memory allocation failed");
    }
    
    puts("User program test completed!");
    return 0;
}