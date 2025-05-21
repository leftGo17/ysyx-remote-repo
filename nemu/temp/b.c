#include <stdio.h>    // For printf
#include <stdlib.h>   // For strtoul
#include <stdint.h>   // For uint32_t

int main() {
    char *arg2 = ""; // 示例字符串
    uint32_t addr = strtoul(arg2, NULL , 10);
    printf("%X\n", addr);
    printf("%d\n", addr);
    return 0; 
}