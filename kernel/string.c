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

int memory_compare(const void* s1, const void* s2, int n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while (n > 0) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
        n--;
    }
    return 0;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    
    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        
        if (!*n) return (char*)haystack;
        haystack++;
    }
    return 0;
}

void strcat(char* dest, const char* src) {
    // Find end of dest string
    while (*dest) dest++;
    
    // Copy src to end of dest
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

char* strrchr(const char* str, char c) {
    const char* last = 0;
    while (*str) {
        if (*str == c) {
            last = str;
        }
        str++;
    }
    return (char*)last;
}