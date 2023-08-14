/**
 * @file    disk.h
 * @author  your name (you@domain.com)
 * @brief   To read and write hard disk using disk controller, we will use LBA
 *          addressing instead CHS addressing.
 * 
 *          The LBA concept erases the definitions of Cylinders and Head in CHS.
 *          The LBA address system sees the drive as a storage entity that holds
 *          consecutive blocks. So it can be said that the LBA address system is
 *          a linear address system, meaning that the first block will have
 *          address 0; and the last block will own the highest address,
 *          depending on the drive size and the area of ​​the block.
 *
 *          Instead of specifying the location of Cylinder, Head and Sector as
 *          in CHS, the disk controller only needs to know the LBA address of a
 *          block to access it easily. Using LBA, the drive can access output
 *          more blocks only, meaning that its capacity may be larger. Note that
 *          the block concept in LBA is equivalent to the sector in CHS.
 * 
 * @version 0.1
 * @date 2023-08-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define SECTOR_SIZE     512

/* Public type ---------------------------------------------------------------*/

/* Public function prototype -------------------------------------------------*/

/**
 * @brief       Read number of sectors from hard disk to memory. Each sector can
 *              be read into memory and is given a LBA (Logic Block Address)
 *              number.
 * 
 * @param[in] lba       - Sector number.
 * @param[in] sectors   - Number of sectors to read.
 * @param[out] buf      - Buffer data.
 * @return int          - Zero if success.
 */
int DiskReadSectors(int lba, int sectors, void *buf);
