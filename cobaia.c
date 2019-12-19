#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(void) {

    printf("pid: %ld\n", (long int) getpid());
    printf("Press <ENTER> to refresh and <CTRL-C> to quit.\n");
    printf("\n\n\n");

    int n = 99;
    char s[] = "in the stack";
    char *h = (char *) malloc(sizeof(char) * 12);
    strcpy(h, "in the heap");

    char curr = '\n';

    do {
        if (curr == '\n') {
            printf("\033[3F");
            printf("%p %d\033[K\n", &n, n);
            printf("%p %s\033[K\n",  s, s);
            printf("%p %s\033[K\n",  h, h);
        }
    } while (curr = getchar());

    free(h);

    return 0;
}

