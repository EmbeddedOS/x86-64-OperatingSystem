#include <stdint.h>
#include <stdio.h>

int Sleep(int time);

int main(void) 
{
    while (1)
    {
        printf("Process 2\n");
        /* SLeep 2 second. */
        Sleep(200);
    }

    return 0;
}