#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "printk.h"
#include "trap.h"
#include "assert.h"
#include "memory.h"
#include "process.h"
#include "syscall.h"
#include "disk.h"
#include "file.h"

void KMain(void)
{
    InitIDT();
    printk("Retrieve memory map:\n");
    RetrieveMemoryInfo();
    InitMemory();

    /* Test file system. Get boot sector. */
    BPB * bpb= kalloc();
    DiskReadSectors(0,1, bpb);
    printk("Volume ID string: %s\n", bpb->volume_id_string);

    InitSystemCall();
    InitProcess();
    StartScheduler();
}