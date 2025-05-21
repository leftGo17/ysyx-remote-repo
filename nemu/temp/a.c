#include <stdio.h>  // For printf
#include <stdint.h> // For uint32_t


uint32_t add(uint32_t a, uint32_t b)
{
    return a + b;
}
int main() {
    uint32_t value1 = 0xFFFFFFFF;
    uint32_t value2 = 0;

    uint32_t ans = add(value1, value2);

    printf("%u\n", ans);
    printf("带0x前缀和填充:\n");
    printf("%#010X\n", ans);

    return 0;
}