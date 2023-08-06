#include <stdint.h>
#include <stdio.h>

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        if (count % 100000000 == 0) {
            printf("Process 3\n");
        }
        count++;
    }

    return 0;
}