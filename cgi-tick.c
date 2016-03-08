#include <stdio.h>
#include <unistd.h>

int main () {
    for (int i = 0; i < 10; i++) {
        printf("tick\n");
        fflush(stdout);
        sleep(1);
    }
    printf("BOOM!\n");
    return 0;
}
