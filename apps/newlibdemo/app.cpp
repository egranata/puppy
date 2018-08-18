#include <newlib/stdio.h>
#include <newlib/stdlib.h>

int main() {
    printf("I am a program that links newlib\n");
    int x = 3;
    printf("Write any integer to continue:\n");
    scanf("%i", &x);
    printf("\nYou wrote %d\n", x);
    FILE* f = fopen("/devices/rtc/now", "r");
    uint8_t *buffer = (uint8_t*)calloc(256, 1);
    fread(buffer, 256, 1, f);
    printf("into buffer %p I read %s\n", buffer, buffer);
    fclose(f);
    exit(5);
}
