#!/bin/bash

# Build script for user programs
# This creates simple C programs and adds them to the filesystem

echo "Building user programs..."

# Create a simple hello world program
cat > /tmp/hello.c << 'EOF'
#include "userlib.h"

int main(void) {
    puts("Hello from user space!");
    puts("This program is running in user mode.");
    puts("It's isolated from the kernel.");
    return 0;
}
EOF

# Create a calculator program
cat > /tmp/calc.c << 'EOF'
#include "userlib.h"

int main(void) {
    puts("Simple Calculator");
    puts("================");
    
    int a = 15;
    int b = 7;
    
    puts("Computing 15 + 7...");
    int sum = a + b;
    
    puts("Computing 15 - 7...");
    int diff = a - b;
    
    puts("Computing 15 * 7...");
    int prod = a * b;
    
    puts("Computing 15 / 7...");
    int quot = a / b;
    
    puts("Results:");
    puts("15 + 7 = 22");
    puts("15 - 7 = 8");
    puts("15 * 7 = 105");
    puts("15 / 7 = 2");
    
    return 0;
}
EOF

# Create a memory test program
cat > /tmp/memtest.c << 'EOF'
#include "userlib.h"

int main(void) {
    puts("Memory Test Program");
    puts("==================");
    
    puts("Testing memory allocation...");
    
    void* ptr1 = sys_malloc(100);
    if (ptr1) {
        puts("Allocated 100 bytes successfully");
    } else {
        puts("Failed to allocate 100 bytes");
    }
    
    void* ptr2 = sys_malloc(200);
    if (ptr2) {
        puts("Allocated 200 bytes successfully");
    } else {
        puts("Failed to allocate 200 bytes");
    }
    
    puts("Freeing memory...");
    sys_free(ptr1);
    sys_free(ptr2);
    puts("Memory freed successfully");
    
    puts("Memory test completed!");
    return 0;
}
EOF

echo "User programs created in /tmp/"
echo "Copy these to your filesystem using the OS commands:"
echo "  save"
echo "  compile hello.c"
echo "  compile calc.c"
echo "  compile memtest.c"
echo "  programs"
echo "  run hello"