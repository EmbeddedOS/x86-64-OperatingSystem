#include <stdint.h>
#include <stddef.h>

void KMain(void)
{
    char* p = (char*)0xB8000;
    p[0] = 'C';
    p[1] = 0xA;
}