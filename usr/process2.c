#include <stdint.h>
#include <stdio.h>

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        if (count % 1000000 == 0) {
            printf("Process 2\n");
        }
        count++;
    }

    return 0;
}