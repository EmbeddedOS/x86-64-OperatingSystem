#pragma once

#include <stddef.h>
#include <stdint.h>
#include "process.h"

/* Public define -------------------------------------------------------------*/
#define STANDARD_INPUT  0
#define STANDARD_OUTPUT 1
#define STANDARD_ERROR  2
#define USER_START_FD   3

#define SECTOR_SIZE     512

/* Public type ---------------------------------------------------------------*/

/**
 * @brief   BIOS parameter block structure.
 * 
 */
typedef struct {
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;

    /* Extended */
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__ ((packed)) BPB;

/**
 * @brief   Directory entry structure.
 * 
 */
typedef struct {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t attr_index;
    uint16_t m_time;
    uint16_t m_date;
    uint16_t cluster_index;
    uint32_t file_size;
} __attribute__ ((packed)) DirEntry;

/**
 * @brief   File control block structure.
 * 
 */
typedef struct {
    char name[8];
    char ext[3];
    uint32_t start_cluster;
    uint32_t dir_entry;
    uint32_t file_size;
    int open_count;
} FCB;

/**
 * @brief   File Descriptor.
 * 
 * @property    fcb         - file descriptor pointer in the process to link
 *                            these two structures.
 * @property    position    - Indicates where the last time the process reads or
 *                            writes in the file.
 */
struct FD {
    FCB *fcb;
    uint32_t position;
    int open_count;
};

typedef struct FD FD;

/* Public function prototype -------------------------------------------------*/
void InitFileSystem(void);

int Open(Process* proc, const char *file_name);
void Close(Process* proc, int fd);
int Read(Process* proc, int fd, void *buffer, int size);
int Lstat(const char *pathname, DirEntry *statbuf);

int GetFileSize(Process *proc, int fd);