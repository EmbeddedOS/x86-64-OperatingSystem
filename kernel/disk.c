#include "disk.h"

/* Public function -----------------------------------------------------------*/
int DiskReadSectors(int lba, int sectors, void *buf)
{
    OutByte(0x1F6, (lba >> 24) | 0b11100000);    /* Port to send drive and bit
                                                  * 24 - 27 of LBA. */
    OutByte(0x1F2, sectors);                     /* Port to send number of
                                                  * sectors. */
    OutByte(0x1F3, (uint8_t)(lba & 0b11111111)); /* Port to send bit 0 - 7 of
                                                  * LBA. */
    OutByte(0x1F4, (uint8_t)(lba >> 8));         /* Port to send bit 8 - 15 of 
                                                  * LBA. */
    OutByte(0x1F5, (uint8_t)(lba >> 16));        /* Port to send bit 16 - 23 of
                                                  * LBA. */
    OutByte(0x1F7, 0x20);                        /* Command port, read with
                                                  * retry. */
                                                 /* 0x20 - READ SECTORS(S) */
                                                 /* 0x30 - WRITE SECTORS(S) */

    /* We are going to read two bytes at a time from the disk controller. */
    uint16_t *ptr = (uint16_t *)buf;

    for (int s = 0; s < sectors; s++)
    {
        /* The sector buffer requires servicing until the sector buffer is
         * ready. */
        int8_t byte = InByte(0x1F7);
        while (!(byte & 0x08))
        {
            byte = InByte(0x1F7);
        }

        /* Copy from hard disk to memory, to read 256 words = 1 sector. */
        for (int i = 0; i < 256; i++)
        {
            *ptr = InWord(0x1F0); /* Data port. */
            ptr++;
        }
    }

    return 0;
}
