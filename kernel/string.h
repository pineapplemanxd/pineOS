#ifndef STRING_H
#define STRING_H

// Custom string functions for bare-metal OS
int strcmp(const char* s1, const char* s2);
int strlen(const char* s);
void strcpy(char* dest, const char* src);
char* strtok(char* str, const char* delim);
char* strchr(const char* str, char c);
int strncmp(const char* s1, const char* s2, int n);
int memory_compare(const void* s1, const void* s2, int n);
char* strstr(const char* haystack, const char* needle);
void strcat(char* dest, const char* src);
char* strrchr(const char* str, char c);

#endif // STRING_H 