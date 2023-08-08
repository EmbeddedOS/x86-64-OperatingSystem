#include <stdint.h>
#include <stdio.h>

int Read(int fd, char *buf, size_t count);

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        printf("User input: ");
        char buffer[10];
        Read(0, buffer, 10);
        printf("Data: %s\n", buffer);
    }

    return 0;
}