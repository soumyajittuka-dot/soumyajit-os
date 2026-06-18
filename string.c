#include "string.h"

void* memcpy(void* dest, const void* src, unsigned int n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for(unsigned int i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void* memset(void* dest, int val, unsigned int n) {
    unsigned char* d = (unsigned char*)dest;
    for(unsigned int i = 0; i < n; i++) {
        d[i] = (unsigned char)val;
    }
    return dest;
}