#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "printk.h"
#include "trap.h"
#include "assert.h"
#include "memory.h"
#include "process.h"
#include "syscall.h"
#include "file.h"

void KMain(void)
{
    InitIDT();
    printk("Retrieve memory map:\n");
    RetrieveMemoryInfo();
    InitMemory();
    InitFileSystem();
    InitSystemCall();
    InitProcess();
    printk("Finished kernel initialization. Welcome to LARVA-OS.\n");
}