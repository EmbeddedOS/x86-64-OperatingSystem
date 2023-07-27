#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "printk.h"
#include "trap.h"
#include "assert.h"

void KMain(void)
{
    InitIDT();
    printk("Hello there!\n");
    printk("%s: %d\n", "Kernel version", 3);
    ASSERT(0);
}