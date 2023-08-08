#include <stdint.h>
#include <stdio.h>

void Waitu(void);

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        if (count % 100000000 == 0) {
         //   printf("Process 1\n");
            Waitu();
        }
        count++;
    }

    return 0;
}