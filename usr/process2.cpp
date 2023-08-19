extern "C" {
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
}

class Test {
    int test;
};

int main(void) 
{


    while (1)
    {
        sleep(100);

        int fd = open("test.txt");
        printf("fd: %d\n", fd);
        char buf[100];

        int byte = read(fd, buf, 100);
        printf("data file is: %s byte: %d\n", buf, byte);

        close(fd);

        printf("closed\n");
    }

    Test t;
    return 0;
}