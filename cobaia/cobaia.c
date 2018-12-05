#include <stdio.h>
#include <termios.h>


int main(void) {

    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &t);

    int a = 99;
    char someString[] = "venomextreme";

    do {
        printf("%p %d\n", &a, a);
        printf("%p %s %c\n", someString, someString, *someString);
    } while (getchar() != 'q');

    return 0;
}
