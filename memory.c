#include "memory.h"
#include "printk.h"
#include "assert.h"

/* Private define ------------------------------------------------------------*/
#define MEMORY_MAX_FREE_REGIONS                 50
#define MEMORY_REGION_USABLE_RAM_TYPE           1
#define MEMORY_REGION_RESERVED_TYPE             2
#define MEMORY_REGION_ACPI_RECLAIMABLE_TYPE     3
#define MEMORY_REGION_ACPI_NVS_TYPE             4
#define MEMORY_REGION_BAD_MEMORY_TYPE           5

/* We save the memory informations at these addresses by using loader code.   */
#define MEMORY_REGION_COUNT_BASE_ADDR           0x9000
#define MEMORY_REGION_STRUCTURES_BASE_ADDR      0x9008

/* Private variable ----------------------------------------------------------*/
static FreeMemoryRegion s_free_memory_regions[MEMORY_MAX_FREE_REGIONS];

/* Public function -----------------------------------------------------------*/
void print_memory_info(void)
{
    int32_t count = *(int32_t *)MEMORY_REGION_COUNT_BASE_ADDR;
    uint64_t total_mem = 0;
    E820 *mem_map = (E820 *)MEMORY_REGION_STRUCTURES_BASE_ADDR;
    int free_memory_region_count = 0;

    ASSERT(count < MEMORY_MAX_FREE_REGIONS);

    for(int32_t i = 0; i < count; i++)
    {
        if(mem_map[i].type == MEMORY_REGION_USABLE_RAM_TYPE) {
            s_free_memory_regions[free_memory_region_count].address =
            mem_map[i].address;

            s_free_memory_regions[free_memory_region_count].length =
            mem_map[i].length;

            total_mem += mem_map[i].length;
            free_memory_region_count++;
        }

        printk("Base: %x   size: %uKB   type: %u\n",
                mem_map[i].address,
                mem_map[i].length/1024,
                (uint64_t)mem_map[i].type);
    }
    printk("Total Free Memory: %uKB\n", total_mem/1024);
}