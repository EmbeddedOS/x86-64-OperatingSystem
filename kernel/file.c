#include <string.h>
#include "file.h"
#include "disk.h"
#include "assert.h"
#include "memory.h"
#include "printk.h"

/* Private define ------------------------------------------------------------*/

/* Private variable ----------------------------------------------------------*/
static BPB s_BIOS_parameter_block = {0};

/* Private function prototype ------------------------------------------------*/
BPB *GetBPB(void);

/* Public function  ----------------------------------------------------------*/
void InitFileSystem(void)
{
    BPB *bpb = (BPB *)kalloc();
    ASSERT(bpb != NULL);

    DiskReadSectors(0, 1, bpb);
    memcpy(&s_BIOS_parameter_block, bpb, sizeof(BPB));
    kfree(bpb);
    printk("OEM identifier: %s\n", s_BIOS_parameter_block.oem_identifier);

    printk("Initialized FAT 16 file system.\n");
}

/* Private function  ---------------------------------------------------------*/
BPB *GetBPB(void)
{
    return &s_BIOS_parameter_block;
}
