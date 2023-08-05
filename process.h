#pragma once
#include <stdint.h>
#include "trap.h"
#include "memory.h"

/* Public define -------------------------------------------------------------*/
#define STACK_SIZE                      PAGE_SIZE    /* 2MB. */
#define MAXIMUM_NUMBER_OF_PROCESS       10
#define USER_STACK_START                (USER_VIRTUAL_ADDRESS_BASE + STACK_SIZE)
/* Public type ---------------------------------------------------------------*/
typedef enum  {
    PROCESS_SLOT_UNUSED = 0,
    PROCESS_SLOT_INITIALIZED
} ProcessState;


/**
 * @brief   Process Control Block structure. This structure is used to store the
 *          essential data of the process. It is maintained in the kernel space,
 *          user program is not allowed to access it.
 *
 * @property pid        - Process Identification number of a process.
 * @property state      - Current state of process
 * @property page_map   - Saves the address of page map level 4 table, when we
 *                        run the process, we use this to switch to the process
 *                        's virtual memory.
 * @property stack      - Stack pointer is used when enter the kernel mode. A
 *                        Process has two stack, one for user code, and one for
 *                        kernel code. The one for user code is saved in trap
 *                        frame.
 * @property tf         - 
 */
typedef struct {
    int pid;
    ProcessState state;
    uint64_t page_map;
    uint64_t stack;
    TrapFrame *tf;
} Process;

/**
 * @brief   The TSS (Task state segment) structure is used only for setting up
 *          stack pointer for ring 0.
 */
typedef struct {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t res1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t res2;
    uint16_t res3;
    uint16_t iopb;
} __attribute__ ((packed)) TSS;

/* Public function prototype -------------------------------------------------*/

void InitProcess(void);
void StartScheduler(void);
void ProcessStart(TrapFrame *tf);