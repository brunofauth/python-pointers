#include <stdio.h>
#include <unistd.h>


int main(void) {

    printf("pid: %ld\n", (long int) getpid());
    printf("Press <ENTER> to refresh and <CTRL-C> to quit.\n\n");

    int n = 99;
    char s[] = "venomextreme";

    char curr;
    printf("%p %d\033[K\n", &n, n);
    printf("%p %s\033[K\n",  s, s);

    while (curr = getchar()) {
        if (curr == '\n') {
            printf("\033[3F");
            printf("%p %d\033[K\n", &a, a);
            printf("%p %s\033[K\n", someString, someString);
        }
    }

    return 0;
}

