/**
 * @file    memory.h
 * @author  Cong Nguyen (congnt264@gmail.com)
 * @brief   This file implement features related to memory management. Paging is
 *          very strong feature of the Intel CPUs. This help we can make much
 *          virtual memory per process, they are isolated with each other and
 *          but share with same kernel virtual memory. Each virtual memory is
 *          managed by a page map structure (we can using it to load virtual
 *          memory). The page map structure divide the virtual memory become
 *          fragmentation are called as pages. Each pages have its attribute
 *          such as writable, readable, present, etc. We can use them to manage
 *          memory access, every violation could cause a CPU exception.
 * 
 *          In our system, we will map the kernel from 0x00 physical address
 *          to KERNEL_VIRTUAL_ADDRESS_BASE with size is all of our free memory.
 *          Actually, the kernel text, rodata, data, bss sections start from
 *          the base + 0x200000. The kernel heap start from l_kernel_end (that
 *          is symbol which mark the end of the sections) to the end of free
 *          memory. The kernel stack start from base + 0x200000 and downward.
 * 
 *          In kernel heap region, we using it to allocate memory for another
 *          features. And user program is one of them, for each request creating
 *          new process, we make a virtual memory with size is one page (2MB)
 *          and reside user program to it. All user virtual memories will refer
 *          to the same virtual address (USER_VIRTUAL_ADDRESS_BASE), but they
 *          are isolated at all (because its physical memory refer to another
 *          region). User virtual memory attributes are writeable, and user to
 *          force them run on ring 3, and no permission to access another
 *          regions. So That is the way how we manage memory. The physical
 *          memory layout:
 *
 *             Physical Memory
 *          |-------------------| -> Max size. From here down to l_kernel_end.
 *          | Free: kernel heap |    We use this for kernel dynamic allocation.
 *          |___________________|    It is used for create user processes, etc.
 *          |_____Process 1_____|
 *          |_____Process 2_____|
 *          | Free: kernel heap |
 *          |_____Process n_____|
 *          |                   |
 *          |                   |
 *          | - - - - - - - - - | -> kernel code end symbol (l_kernel_end).
 *          | Free: kernel bss  |
 *          | - - - - - - - - - |
 *          | Free: kernel data |
 *          | - - - - - - - - - | -> After switch to long mode, we relocate the
 *          | Free: kernel text |    kernel code from 0x100000 to 0x00200000.
 *          | - - - - - - - - - | 0x00200000 -> In LM, we load stack start from
 *          | Free: kernel stack|               here and grow downward.
 *          |                   |
 *          |                   |
 *          |                   | -> In loader code, we load kernel code from
 *          |                   |    disk to 0x00100000.
 *          |-------------------| 0x00100000
 *          |      Reserved     |
 *          |-------------------| 0x80000
 *          |      Free         | -> We use this to save some memory information
 *          |                   |    which is provided by BIOS service.
 *          |-------------------|
 *          |      Loader       | 0x7E00
 *          |-------------------|
 *          |     MBR code      | 0x7C00
 *          |-------------------|
 *          |      Free         | -> We use this section as stack when we are
 *          |                   |    running in loader code: 16, 32 bit modes.
 *          |-------------------|
 *          | BIOS data vectors |
 *          |-------------------| 0
 *
 *          Kernel virtual memory layout map to physical memory:
 *               Kernel virtual memory              Physical Memory
 *               |KVBase + MaxFreeMem|<----------->|    MaxFreeMem   |
 *               |      heap         |             |                 |
 *               |      heap         |             |                 |
 *               |      bss          |             |                 |
 *               |      data         |             |                 |
 *               |      text         |             |                 |
 *               |0xFFFF800000200000 |<----------->|    0x00200000   |
 *               |      stack        |             |                 |
 *         KVBase|0xFFFF800000000000 |<----------->|         0       |
 * 
 *          Process virtual memory map to physical memory. The physical memory
 *          address that is mapped to, depends on current free memory address
 *          of the kernel heap:
 *                                   Physical Memory    Kernel virtual memory
 *                                   | MaxFreeMem|     |KVBase + MaxFreeMem  |
 *                                   |           |     |                     |
 *          Process Virtual memory   |           |     |                     |
 *           |0x400000+PageSize|<--->|-----------|<--->|---------------------|
 *           |                 |     |  A page   |     |Some kernel free page|
 *           |    0x400000     |<--->|-----------|<--->|---------------------|
 *                                   |           |     |                     |
 *                                   |           |     |                     |
 *                                   |    0      |     | 0xFFFF800000000000  |

 * @version 0.1
 * @date 2023-07-21
 * 
 * @copyright Copyright (c) 2023
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

/* Public define -------------------------------------------------------------*/
#define PAGE_SIZE                   (2 * 1024 * 1024)   /* 2MB.               */
#define KERNEL_VIRTUAL_ADDRESS_BASE 0xFFFF800000000000
#define USER_VIRTUAL_ADDRESS_BASE   0x400000
#define PHYSICAL_MEMORY_SIZE        0x40000000    /* 1GB. TODO: extend RAM.   */
#define VIRTUAL_ADDRESS_END         (KERNEL_VIRTUAL_ADDRESS_BASE + \
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
#define PHY_TO_VIR(p)       ((uint64_t)(p)+KERNEL_VIRTUAL_ADDRESS_BASE)
#define VIR_TO_PHY(v)       ((uint64_t)(v)-KERNEL_VIRTUAL_ADDRESS_BASE)

/**
 * @def Macros provide page table attributes.
 */
#define TABLE_ENTRY_PRESENT_ATTRIBUTE       BIT(0)
#define TABLE_ENTRY_WRITABLE_ATTRIBUTE      BIT(1)
#define TABLE_ENTRY_USER_ATTRIBUTE          BIT(2)
#define TABLE_ENTRY_ENTRY_ATTRIBUTE         BIT(7)

/**
 * @def Macros retrieve page table entry addresses by clear attributes bit.
 */
#define PAGE_MAP_LV4_TABLE_ADDRESS(p)               (((uint64_t)p >> 12) << 12)
#define PAGE_DIRECTORY_POINTER_TABLE_ADDRESS(p)     (((uint64_t)p >> 12) << 12)
#define PAGE_DIRECTORY_TABLE_ADDRESS(p)             (((uint64_t)p >> 12) << 12)
#define PAGE_ADDRESS(p)                             (((uint64_t)p >> 21) << 21)

#define ADDR_IS_ALIGNED(a)              (((uint64_t)a % PAGE_SIZE) == 0)
#define ASSERT_ADDR_IS_ALIGNED(a)       ASSERT(ADDR_IS_ALIGNED(a))
 
/* Each memory map have 512 page directory pointer tables. */
#define TOTAL_PAGE_DIR_POINTER_TABLE                512
/* Each PDP table also include 512 entries which point to page directory tables.
 */
#define TOTAL_PAGE_DIR_TABLE_OF_EACH_PDPT           512

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

/**
 * @brief   Page Directory Pointer Table point to Page Directory, Page Directory
 *          point to Page Directory Entry, etc.
 */
typedef uint64_t PageDirEntry;
typedef PageDirEntry* PageDir;
typedef PageDir* PageDirPointerTable;

/* Public function prototype -------------------------------------------------*/
void LoadCR3(uint64_t map);
void RetrieveMemoryInfo(void);
void InitMemory(void);
void SwitchVM(uint64_t map);

/**
 * @brief Create new virtual memory for user program. Currently, we only support
 *        one page for each user program memory space. Every user program will
 *        using same base virtual memory address (USER_VIRTUAL_ADDRESS_BASE) but
 *        in physical memory, that refer to another memory regions (random free 
 *        pages) and also not effect to kernel memory.
 *        In user virtual memory, kernel pages are always resided at 
 *        (KERNEL_VIRTUAL_ADDRESS_BASE) address and share between all virtual
 *        memory. 
 * 
 * @param map           - Page map level 4 table.
 * @param start         - Start location of user program. 
 * @param size          - size of user program.
 * @return true         - Create a VM success and mapping to the map.
 * @return false 
 */
bool SetupUVM(uint64_t map, uint64_t start_location, int size);

void FreeVM(uint64_t map);

void kfree(uint64_t addr);

/**
 * @brief Allocate a page size memory for caller, this function return a virtual
 *        address base.
 * 
 * @return void*        - Virtual memory base address.
 *                      - NULL if failed.
 */
void* kalloc(void);