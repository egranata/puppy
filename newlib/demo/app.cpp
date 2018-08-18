#include <newlib/stdio.h>
#include <newlib/stdlib.h>

int main() {
    printf("I am a program that links newlib\n");
    int x = 3;
    printf("Write any integer to continue:\n");
    scanf("%i", &x);
    printf("\nYou wrote %d\n", x);
    exit(5);
}
