#include <stdint.h>
#include <stdio.h>

void Exit(void);

int Sleep(int time);

int main(void) 
{
    int64_t count = 0;

    while (1)
    {
       // printf("Process 2\n");
        /* SLeep 1 second. */
        // Sleep(100);
        // count++;

        // if (count == 10) {
        //    // printf("Emit exception by access to kernel code.\n");
        //     char *p = (char *)0xFFFF800000300400;
        //     *p = 1;
        //     break;
        // }
    }

    return 0;
}