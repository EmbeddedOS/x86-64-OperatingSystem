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
static FCB *s_fcb_table = NULL;
static FD *s_fd_table = NULL;

/* Private function prototype ------------------------------------------------*/
BPB *GetBPB(void);

int FindFileInRootDir(const char *filename, DirEntry* entry);

static void GetRelativeFileName(DirEntry* entry, char *buf);

static void ReadFileData(int start_cluster, int length, void *buf);

/**
 * @brief   We allocate a memory page for FCB table, so the maximum entries of
 *          this table is PAGE_SIZE / sizeof(FCB).
 * 
 */
void InitFileControlBLock(void);

/**
 * @brief   We allocate a memory page for FD table, so the maximum entries of
 *          this table is PAGE_SIZE / sizeof(FD).
 * 
 */
void InitFileDescriptorTable(void);

static inline int GetMaxEntriesOfFCBTable()
{
    return PAGE_SIZE /sizeof(FCB);
}

static inline int GetMaxEntriesOfFDTable()
{
    return PAGE_SIZE /sizeof(FD);
}

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

static int
ReadRawData(uint32_t cluster_index, char *buf, uint32_t pos, uint32_t size);

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

    /* 3. Initialize File control block table and file descriptor table. */
    InitFileControlBLock();
    InitFileDescriptorTable();

    printk("Initialized FAT 16 file system.\n");
}

int Open(Process* proc, const char *file_name)
{
    int fd = -1;
    int file_desc_index = -1;
    int entry_index = 0;

    /* 1. Find a file entry in the process. */
    for (int i = USER_START_FD; i < PROCESS_MAXIMUM_FILE_DESCRIPTOR; i++) {
        if (proc->file[i] == NULL) {
            fd = i;
            break;
        }
    }

    if (fd == -1) {
        /* The process opened maximum files. */
        return -1;
    }

    /* 2. Find the table entry for FD. */
    for (int i = 0; i < GetMaxEntriesOfFDTable(); i++) {
        if (s_fd_table[i].fcb == NULL) {
            file_desc_index = i;
            break;
        }
    }

    if (file_desc_index == -1) {
        /* No entry available. */
        return -1;
    }

    /* 3. Find the file on the disk. And we use the entry index for the file
          control block index and file descriptor index also. */
    DirEntry entry = {0};
    entry_index = FindFileInRootDir(file_name, &entry);
    if (entry_index < 0) {
        /* Not found the file on the disk. */
        return -1;
    }

    if (entry.cluster_index < START_CLUSTER_INDEX) {
        /* Sometime, when the file is just created, we can find it in root
         * directory, but the data is not wrote to data section yet, so cluster
         * index maybe is 0. In this case we will send a error to user.
         * TODO: Return a try again error code. */
        return -1;
    }

    /* 4. Update file control block entry. */
    if (s_fcb_table[entry_index].open_count == 0) {
        /* If this file is not opened yet, we setup the entry in FCB table. */
        s_fcb_table[entry_index].dir_entry = entry_index;
        s_fcb_table[entry_index].file_size = entry.file_size;
        s_fcb_table[entry_index].start_cluster = entry.cluster_index;

        memcpy(&s_fcb_table[entry_index].name, &entry.name, 8);
        memcpy(&s_fcb_table[entry_index].ext, &entry.ext, 3);
    }

    s_fcb_table[entry_index].open_count++;

    /* 5. link file descriptor entry to the FCB entry. */
    memset(&s_fd_table[file_desc_index], 0, sizeof(FD));
    s_fd_table[file_desc_index].fcb = &s_fcb_table[entry_index];
    s_fd_table[file_desc_index].open_count = 1;

    /* 6. Link the process file descriptor to the file descriptor entry. */
    proc->file[fd] = &s_fd_table[file_desc_index];

    return fd;
}

void Close(Process* proc, int fd)
{
    if (proc->file[fd] == NULL) {
        return;
    }

    ASSERT (proc->file[fd]->fcb->open_count > 0);
    proc->file[fd]->fcb->open_count--;
    proc->file[fd]->open_count--;

    /* We don't clear file control block, because, when the file is opened, the
     * file data is cached in the table, and then we can easily retrieve th file
     * info. */
    if (proc->file[fd]->open_count == 0) {
        /* If the FD count is zero, mean fd entry is not used, we save it to
         * NULL. Otherwise, the file descriptor entry is used by others and we
         * leave the FCB pointer unchanged. */
        proc->file[fd]->fcb = NULL;
    }

    proc->file[fd] = NULL;
}

int Read(Process* proc, int fd, void *buffer, int size)
{
    uint32_t read_size;

    if (proc->file[fd] == NULL) {
        return -1;
    }

    uint32_t position = proc->file[fd]->position;
    uint32_t file_size = proc->file[fd]->fcb->file_size;

    if (position + size > file_size) {
        /* Read the rest of file. */
        size = file_size - position;
    }

    read_size = ReadRawData(proc->file[fd]->fcb->start_cluster,
                            buffer,
                            position,
                            size);

    proc->file[fd]->position += read_size;

    return read_size;
}

int GetFileSize(Process *proc, int fd)
{
    if (proc->file[fd] == NULL) {
        return -1;
    }

    return proc->file[fd]->fcb->file_size;
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
    ASSERT(start_cluster >= START_CLUSTER_INDEX);
    uint32_t file_data_start_sector = (start_cluster - START_CLUSTER_INDEX)
                                      * GetSectorsPerCluster()
                                      + GetDataRegionStartSector();

    uint16_t number_of_sector_need_to_read = length
                                             * GetSectorsPerCluster();

    DiskReadSectors(file_data_start_sector,
                    number_of_sector_need_to_read,
                    buf);
}

void InitFileControlBLock(void)
{
    s_fcb_table = (FCB *)kalloc();
    ASSERT(s_fcb_table);

    memset(s_fcb_table, 0, PAGE_SIZE);
}

void InitFileDescriptorTable(void)
{
    s_fd_table = (FD *)kalloc();
    ASSERT(s_fd_table);

    memset(s_fd_table, 0, PAGE_SIZE);
}

static int
ReadRawData(uint32_t cluster_index, char *buf, uint32_t pos, uint32_t size)
{
    uint16_t number_of_clusters_need_to_read
        = GetNumberOfClustersStoringFileData(size);

    uint16_t start_cluster_need_to_read = cluster_index
                                          + pos / GetBytesPerCluster();

    uint16_t start_pos_in_cluster = pos % GetBytesPerCluster();

    char *buffer = (char *)kalloc();

    ReadFileData(start_cluster_need_to_read,
                 number_of_clusters_need_to_read,
                 buffer);

    memcpy(buf, &buffer[start_pos_in_cluster], size);

    kfree(buffer);

    return size;
}