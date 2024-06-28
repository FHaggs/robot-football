#include <stdio.h>

int abs(int x) {
    int mask = x >> (sizeof(int) * 8 - 1);
    return (x + mask) ^ mask;
}

int main() {
    int value = -5;
    int absValue = abs(value);

    printf("The absolute value of %d is %d\n", value, absValue);

    return 0;
}

