#include <string.h>
#include <strings.h>

#include "file.h"
#include "disk.h"
#include "assert.h"
#include "memory.h"
#include "printk.h"

/* Private define ------------------------------------------------------------*/
#define ENTRY_EMPTY         0
#define ENTRY_DELETED       0xE5


/* Private variable ----------------------------------------------------------*/
static BPB s_BIOS_parameter_block = {0};
static uint64_t s_data_region_base_address = {0};
static struct DirEntryRoot {
    uint64_t base_address;
    uint32_t total_entries;
} s_dir_entry_root = {0};

/* Private function prototype ------------------------------------------------*/
BPB *GetBPB(void);
int FindFileInRootDir(const char *filename, DirEntry* entry);
static void GetRelativeFileName(DirEntry* entry, char *buf);
/* Public function  ----------------------------------------------------------*/
void InitFileSystem(void)
{
    /* 1. Get BIOS parameter block and validate signatures. */
    BPB *bpb = (BPB *)kalloc();
    ASSERT(bpb != NULL);

    DiskReadSectors(0, 1, bpb);
    memcpy(&s_BIOS_parameter_block, bpb, sizeof(BPB));

    uint8_t boot_signature_1 = (uint8_t)((char *)bpb)[510];
    uint8_t boot_signature_2 = (uint8_t)((char *)bpb)[511];
    uint16_t boot_signature = (((uint16_t)boot_signature_1) << 8)
                                | boot_signature_2;

    uint8_t fat16_signature = s_BIOS_parameter_block.signature;

    printk("Boot signature: %x\n", boot_signature);
    printk("FAT16 signature: %x\n", fat16_signature);

    ASSERT(boot_signature == 0x55AA);
    ASSERT(fat16_signature == 0x29);

    kfree(bpb);
    printk("OEM identifier: %s\n", s_BIOS_parameter_block.oem_identifier);

    /* 2. Calculate root directory address. */
    s_dir_entry_root.base_address = (s_BIOS_parameter_block.fat_copies
                                    * s_BIOS_parameter_block.sectors_per_fat
                                    + s_BIOS_parameter_block.reserved_sectors)
                                    * s_BIOS_parameter_block.bytes_per_sector;

    s_dir_entry_root.total_entries = s_BIOS_parameter_block.root_dir_entries;

    uint64_t root_directory_size = s_BIOS_parameter_block.root_dir_entries
                                    * sizeof(DirEntry);

    s_data_region_base_address = s_dir_entry_root.base_address
                                 + root_directory_size;

    printk("FAT16 Root Directory base address: %x\n",
            s_dir_entry_root.base_address);

    printk("FAT16 DATA region base address: %x\n",
            s_data_region_base_address);

    DirEntry entry;
    int entry_number = FindFileInRootDir("test.txt", &entry);
    if (entry_number >= 0) {
        printk("Found this file test.txt at entry %d\n", entry_number);
        printk("Data at cluster %d\n", entry.cluster_index);
    }

    printk("Initialized FAT 16 file system.\n");
}

/* Private function  ---------------------------------------------------------*/
BPB *GetBPB(void)
{
    return &s_BIOS_parameter_block;
}

int FindFileInRootDir(const char *filename, DirEntry* entry)
{
    int status = -1;

    DirEntry *sector_data= kalloc();
    uint16_t number_of_sectors = GetBPB()->root_dir_entries
                                * sizeof(DirEntry)
                                / GetBPB()->bytes_per_sector;
    uint32_t root_entry_start = (GetBPB()->fat_copies
                                 * GetBPB()->sectors_per_fat
                                 + GetBPB()->reserved_sectors);

    uint16_t entries_per_sector = GetBPB()->bytes_per_sector / sizeof(DirEntry);
    for (int i = 0; i < number_of_sectors; i++) {
        
        /* Read 1 sector a time. */
        memset(sector_data, 0, GetBPB()->bytes_per_sector);

        DiskReadSectors(root_entry_start + i, 1, sector_data);

        for (int j = 0; j < entries_per_sector; j++) {
            if (sector_data[j].name[0] == ENTRY_EMPTY 
                || sector_data[j].name[0] == ENTRY_DELETED) {
                continue;
            }

            if (sector_data[j].attributes == 0xF) {
                /* We don't support long file name. */
                continue;
            }

            char tmp_filename[13] = {0};
            GetRelativeFileName(&sector_data[j], tmp_filename);

            if (strcasecmp(filename, tmp_filename) == 0) {
                /* Found the file. */
                memcpy(entry, &sector_data[j], sizeof(DirEntry));
                status = i * entries_per_sector + j;
                goto exit;
            }
        }
    }

exit:
    kfree(sector_data);
    return status;
}


static void GetRelativeFileName(DirEntry* entry, char *buf)
{
    char *filename = entry->name;
    char *ext = entry->ext;
    int i = 0;

    while (*filename != '\0' && *filename != ' ') {
        *buf = *filename;
        filename++;
        buf += 1;

        if (i >= sizeof(entry->name) - 1) {
            break;
        }

        i++;
    }

    *buf = '.';
    buf += 1;
    i = 0;

    while (*ext != '\0' && *ext != ' ')
    {
        *buf = *ext;
        ext++;
        buf += 1;

        if (i >= sizeof(entry->ext) - 1) {
            break;
        }

        i++;
    }

    *buf = 0x00;
}
