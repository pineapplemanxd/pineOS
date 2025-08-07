#include "userlib.h"

// System call wrapper functions
// These would normally use inline assembly to trigger interrupts

int sys_exit(int code) {
    // For now, just return - in real implementation would trigger syscall
    return 0;
}

int sys_write(int fd, const char* buffer, int count) {
    // Placeholder - would trigger syscall interrupt
    return count;
}

int sys_read(int fd, char* buffer, int count) {
    // Placeholder - would trigger syscall interrupt
    return 0;
}

void* sys_malloc(int size) {
    // Placeholder - would trigger syscall interrupt
    return (void*)0;
}

void sys_free(void* ptr) {
    // Placeholder - would trigger syscall interrupt
}

// Standard library implementations
int printf(const char* format, ...) {
    // Simple printf implementation
    const char* p = format;
    while (*p) {
        if (*p == '%' && *(p+1)) {
            p += 2; // Skip format specifier for now
        } else {
            sys_write(1, p, 1);
            p++;
        }
    }
    return 0;
}

int puts(const char* str) {
    int len = strlen(str);
    sys_write(1, str, len);
    sys_write(1, "\n", 1);
    return len + 1;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

// Default program entry point
int main(void) {
    puts("Hello from user space!");
    return 0;
}