#ifndef USERLIB_H
#define USERLIB_H

// User library for C programs running in user space

// System call interface
int sys_exit(int code);
int sys_write(int fd, const char* buffer, int count);
int sys_read(int fd, char* buffer, int count);
void* sys_malloc(int size);
void sys_free(void* ptr);

// Standard library functions
int printf(const char* format, ...);
int puts(const char* str);
int strlen(const char* str);
int strcmp(const char* s1, const char* s2);
void strcpy(char* dest, const char* src);

// Program entry point
int main(void);

#endif