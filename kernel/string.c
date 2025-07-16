#include "string.h"

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

char* strtok(char* str, const char* delim) {
    static char* saved_str = 0;
    if (str) {
        saved_str = str;
    }
    if (!saved_str) {
        return 0;
    }
    while (*saved_str && strchr(delim, *saved_str)) {
        saved_str++;
    }
    if (!*saved_str) {
        return 0;
    }
    char* token = saved_str;
    while (*saved_str && !strchr(delim, *saved_str)) {
        saved_str++;
    }
    if (*saved_str) {
        *saved_str = '\0';
        saved_str++;
    } else {
        saved_str = 0;
    }
    return token;
}

char* strchr(const char* str, char c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return 0;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n > 0 && *s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *s1 - *s2;
} 