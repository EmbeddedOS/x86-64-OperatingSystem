#include <stdint.h>
#include <stdio.h>

void Waitu(void);

int main(void) 
{
    int64_t count = 0;
    while (1)
    {
        if (count % 100000000 == 0) {
            Waitu();
        }
        count++;
    }

    return 0;
}