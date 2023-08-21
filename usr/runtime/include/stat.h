#pragma once

#include <stddef.h>
#include <stdint.h>


/* Public define -------------------------------------------------------------*/
#define FILE_ATTR_READ_ONLY      0b00000001
#define FILE_ATTR_HIDDEN         0b00000010
#define FILE_ATTR_SYSTEM         0b00000100
#define FILE_ATTR_VOLUME_LABEL   0b00001000
#define FILE_ATTR_DIRECTORY      0b00010000
#define FILE_ATTR_ARCHIVE        0b00100000

/* Public type ---------------------------------------------------------------*/
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
} __attribute__ ((packed)) stat;

/* Public function prototype -------------------------------------------------*/
int lstat(const char *pathname, stat *statbuf);