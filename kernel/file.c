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
#define START_CLUSTER_INDEX 2

/* Private variable ----------------------------------------------------------*/
static BPB s_BIOS_parameter_block = {0};

/* Private function prototype ------------------------------------------------*/
BPB *GetBPB(void);

int FindFileInRootDir(const char *filename, DirEntry* entry);

static void GetRelativeFileName(DirEntry* entry, char *buf);

static void ReadFileData(int start_cluster, int length, void *buf);

static inline int GetRootDirectoryStartSector(void)
{
    return GetBPB()->fat_copies
           * GetBPB()->sectors_per_fat
           + GetBPB()->reserved_sectors;
}

static inline int GetRootDirectorySectorSize(void)
{
    return GetBPB()->root_dir_entries
           * sizeof(DirEntry)
           / GetBPB()->bytes_per_sector;
}

static inline int GetDataRegionStartSector(void)
{
    return GetRootDirectoryStartSector()
           + GetRootDirectorySectorSize();
}

static inline int GetBytesPerSector(void)
{
    return GetBPB()->bytes_per_sector;
}

static inline int GetSectorsPerCluster(void)
{
    return GetBPB()->sectors_per_cluster;
}

static inline int GetBytesPerCluster(void)
{
    return GetSectorsPerCluster() * GetBytesPerSector();
}

static inline int GetNumberOfClustersStoringFileData(int file_size)
{
    int length = file_size / GetBytesPerCluster();
    
    if (file_size % GetBytesPerCluster()) {
        length += 1;
    }

    return length;
}

static inline int GetSignature(void)
{
    return GetBPB()->signature;
}

/* Public function  ----------------------------------------------------------*/
void InitFileSystem(void)
{
    /* 1. Get BIOS parameter block and validate signatures. */
    BPB *bpb = (BPB *)kalloc();
    ASSERT(bpb != NULL);

    DiskReadSectors(0, 1, bpb);
    memcpy(GetBPB(), bpb, sizeof(BPB));

    uint8_t boot_signature_1 = (uint8_t)((char *)bpb)[510];
    uint8_t boot_signature_2 = (uint8_t)((char *)bpb)[511];
    uint16_t boot_signature = (((uint16_t)boot_signature_1) << 8)
                                | boot_signature_2;

    printk("Boot signature: %x\n", boot_signature);
    printk("FAT16 signature: %x\n", GetSignature());

    ASSERT(boot_signature == 0x55AA);
    ASSERT(GetSignature() == 0x29);

    kfree(bpb);

    /* 2. Calculate root directory address. */
    printk("FAT16 Root Directory base address: %x\n",
           GetRootDirectoryStartSector() * GetBytesPerSector());

    printk("FAT16 DATA region base address: %x\n",
            GetDataRegionStartSector() * GetBytesPerSector());

    DirEntry entry;
    int entry_number = FindFileInRootDir("test.txt", &entry);
    if (entry_number >= 0) {

        char buf[2048] = {0};
        ReadFileData(entry.cluster_index,
                     GetNumberOfClustersStoringFileData(entry.file_size),
                     buf);

        printk("Data of file is: %s\n",
               buf,
               entry.cluster_index);
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

    uint16_t number_of_sectors = GetRootDirectorySectorSize();

    uint32_t root_entry_start = GetRootDirectoryStartSector();

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

    *buf = '\0';
}

static void ReadFileData(int start_cluster, int length, void *buf)
{
    /* Note that cluster start with index 2, so we need subtract to 2. */
    ASSERT(start_cluster >= 2);
    uint32_t file_data_start_sector = (start_cluster - START_CLUSTER_INDEX)
                                      * GetSectorsPerCluster()
                                      + GetDataRegionStartSector();

    uint16_t number_of_sector_need_to_read = length
                                             * GetSectorsPerCluster();

    DiskReadSectors(file_data_start_sector,
                    number_of_sector_need_to_read,
                    buf);
}
