#include <stdint.h>
#include <stdio.h>

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        if (count % 10000000 == 0) {
            printf("Process 1\n");
        }
        count++;
    }

    return 0;
}