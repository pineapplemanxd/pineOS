#include "userlib.h"

int main(void) {
    puts("Hello from user space!");
    puts("This is a user program running separately from the kernel.");
    return 0;
}