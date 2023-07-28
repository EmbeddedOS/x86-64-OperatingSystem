#pragma once
#include <stdint.h>

/* Public type ---------------------------------------------------------------*/
/**
 * @brief       The BIOS function: INT 0x15, EAX = 0x820 is detecting upper
 *              memory service. This return a structures data to base address
 *              and increase point to next memory region automatically.
 * @property    address     - First uint64_t = Base address.
 * @property    length      - Second uint64_t = Length of "region" (if this
 *                            value is 0, ignore the entry).
 * @property    type        - Next uint32_t = Region "type".
 *                              - Type 1: Usable (normal) RAM.
 *                              - Type 2: Reserved - unusable.
 *                              - Type 3: ACPI reclaimable memory.
 *                              - Type 4: ACPI NVS memory.
 *                              - Type 5: Area containing bad memory.
 */
typedef struct {
    uint64_t address;       /* Base address of the memory region.   */
    uint64_t length;        /* Length of the memory region.         */
    uint32_t type;          /* Memory type.                         */
} __attribute__ ((packed)) E820;

typedef struct {
    uint64_t address;
    uint64_t length;
} FreeMemoryRegion;

/* Public function prototype -------------------------------------------------*/
void print_memory_info(void);