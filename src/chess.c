#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("This program has %d parameters: ", argc);
    for (int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}
