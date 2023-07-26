#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "printk.h"
#include "trap.h"

void KMain(void)
{
    InitIDT();
    printk("Hello there!\n");
    printk("%s: %d\n", "Kernel version", 3);
}