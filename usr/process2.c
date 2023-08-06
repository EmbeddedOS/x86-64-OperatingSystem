#include <stdint.h>
#include <stdio.h>

void Exit(void);

int Sleep(int time);

int main(void) 
{
    int64_t count = 0;

    while (1)
    {
        printf("Process 2\n");
        /* SLeep 2 second. */
        Sleep(100);
        count++;

        if (count == 10) {
            printf("Exiting... \n");
            break;
        }
    }

    return 0;
}