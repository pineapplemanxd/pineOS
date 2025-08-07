#include "userlib.h"

int main(void) {
    puts("Simple Calculator");
    puts("================");
    
    // Simple arithmetic demonstration
    int a = 10;
    int b = 5;
    
    puts("Calculating 10 + 5...");
    int sum = a + b;
    
    puts("Calculating 10 - 5...");
    int diff = a - b;
    
    puts("Calculating 10 * 5...");
    int prod = a * b;
    
    puts("Calculating 10 / 5...");
    int quot = a / b;
    
    puts("Results:");
    printf("10 + 5 = %d\n", sum);
    printf("10 - 5 = %d\n", diff);
    printf("10 * 5 = %d\n", prod);
    printf("10 / 5 = %d\n", quot);
    
    return 0;
}