#pragma once
#include <stdint.h>
#include <stddef.h>

/* Public define -------------------------------------------------------------*/
#define PAGE_SIZE                   (2 * 1024 * 1024)   /* 2MB.               */
#define VIRTUAL_ADDRESS_BASE        0xFFFF800000000000
#define PHYSICAL_MEMORY_SIZE        0x40000000    /* 1GB. TODO: extend RAM.   */
#define VIRTUAL_ADDRESS_END         (VIRTUAL_ADDRESS_BASE + \
                                     PHYSICAL_MEMORY_SIZE)
/**
 * @def Macro align the address to the next 2MB boundary if it is not align. We
 * simply add a page size and shift right 21 bits and then shift left. Which
 * will clear the 21 bits of the value and now we get aligned address.
 */
#define PAGE_ALIGN_UP(v)    ((((uint64_t)v + PAGE_SIZE - 1)>>21)<<21)

/**
 * @def Macro align the address to the previous 2MB boundary if it is not align.
 */
#define PAGE_ALIGN_DOWN(v)  (((uint64_t)v>>21)<<21)

/**
 * @def Macros convert between virtual address and physical address.
 */
#define PHY_TO_VIR(p)       ((uint64_t)(p)+VIRTUAL_ADDRESS_BASE)
#define VIR_TO_PHY(v)       ((uint64_t)(v)-VIRTUAL_ADDRESS_BASE)

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

struct Page {
    struct Page* next;
};

typedef struct Page Page;

/* Public function prototype -------------------------------------------------*/
void retrieve_memory_info(void);

void kfree(uint64_t addr);
void* kalloc(void);