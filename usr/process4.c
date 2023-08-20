#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void) 
{
    while (1)
    {
        printf("process4 is running.\n");
        sleep(300);
    }    
}