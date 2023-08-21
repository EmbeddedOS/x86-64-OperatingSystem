extern "C" {
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
}

int x = 9;
class Test {

public:
    Test() {
        x = 20;
        printf("=======Constructor.===========\n");
    }

    int test;
};

Test test;

int main(void) 
{
    test.test = 10;
    printf("Test is created, x= %d\n", x);

    while (1)
    {
        sleep(300);

        int fd = open("test.txt");
        //printf("fd: %d\n", (int64_t)fd);

        if (fd < 0) {
        //    printf("Cannot open file\n");
            continue;
        }
        char buf[100];

        int byte = read(fd, buf, 100);
        //printf("data file is: %s byte: %d\n", buf, byte);

        close(fd);

        //printf("closed\n");
    }

    Test t;
    return 0;
}