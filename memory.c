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
extern char l_kernel_end;
static Page s_free_memory_page_head;
static uint64_t s_free_memory_end_address;

/* Private function prototypes -----------------------------------------------*/
static void free_region(uint64_t v_start, uint64_t v_end);

/* Public function -----------------------------------------------------------*/
void retrieve_memory_info(void)
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

        printk("Physical Address: %x   size: %uKB   type: %u\n",
                mem_map[i].address,
                mem_map[i].length/1024,
                (uint64_t)mem_map[i].type);
    }
    printk("Total Free Memory: %uKB\n", total_mem/1024);

    for (int i = 0; i < free_memory_region_count; i++)
    {
        uint64_t v_start = PHY_TO_VIR(s_free_memory_regions[i].address);
        uint64_t v_end = v_start + s_free_memory_regions[i].length;

        /* We collect the free memory. */
        if (v_start > (uint64_t)&l_kernel_end) {
            free_region(v_start, v_end);
        } else if (v_end > (uint64_t)&l_kernel_end) {
            free_region((uint64_t)&l_kernel_end, v_end);
        }
    }

    s_free_memory_end_address = (uint64_t)s_free_memory_page_head.next 
                                + PAGE_SIZE;

    Page* start_page = NULL;
    for (Page *page = s_free_memory_page_head.next; page != NULL;) {
        if (page != NULL) {
            start_page = page;
        }
        page = page->next;
    }

    printk("Virtual Free Memory: %x->%x\n",
            start_page,
            s_free_memory_end_address);
}

/* Private function ----------------------------------------------------------*/
static void free_region(uint64_t v_start, uint64_t v_end)
{
    for (uint64_t start = PAGE_ALIGN_UP(v_start);
            start + PAGE_SIZE <= v_end;
            start += PAGE_SIZE) {

        if (v_start + PAGE_SIZE <= VIRTUAL_ADDRESS_END)
        {
            kfree(start);
        }
    }
}

void kfree(uint64_t addr)
{
    /* Check the address is aligned. */
    ASSERT(addr % PAGE_SIZE == 0);

     /* Check the address is not within kernel and not out of memory. */
    ASSERT(addr >= (uint64_t)&l_kernel_end);
    ASSERT(addr + PAGE_SIZE <= VIRTUAL_ADDRESS_END);

    /* To free memory, we just add it to the free memory page linked list. */
    Page *page_address = (Page *)addr;
    page_address->next = s_free_memory_page_head.next;
    s_free_memory_page_head.next = page_address;
}

void* kalloc(void)
{
    Page *page_address = s_free_memory_page_head.next;

    if (page_address != NULL) {
        /* Check the address is aligned. */
        ASSERT((uint64_t)page_address % PAGE_SIZE == 0);

        /* Check the address is not within kernel and not out of memory. */
        ASSERT((uint64_t)page_address >= (uint64_t)&l_kernel_end);
        ASSERT((uint64_t)page_address + PAGE_SIZE <= VIRTUAL_ADDRESS_END);

        s_free_memory_page_head.next = page_address->next;
    }

    return (void *)page_address;
}