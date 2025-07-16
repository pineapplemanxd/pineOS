#ifndef STRING_H
#define STRING_H

// Custom string functions for bare-metal OS
int strcmp(const char* s1, const char* s2);
int strlen(const char* s);
void strcpy(char* dest, const char* src);
char* strtok(char* str, const char* delim);
char* strchr(const char* str, char c);
int strncmp(const char* s1, const char* s2, int n);

#endif // STRING_H 