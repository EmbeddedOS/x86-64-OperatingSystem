/**
 * @file    process.h
 * @author  Cong Nguyen (congnt264@gmail.com)
 * @brief   A process is a program in execution. In our system we limit at 
 *          `MAXIMUM_NUMBER_OF_PROCESS` process. To schedule process, we create
 *          a structure that is Scheduler to manage them. We use the round-robin
 *          scheduling mechanism in our system, we achieved that by using the
 *          timer interrupt, the timer handler we be called every 10ms, so in
 *          this, we perform context switch between processes.
 * 
 *          The scheduler structure maintain three queues: ready queue, waiting
 *          queue and killed queue.
 *          + The ready queue to push and pop ready processes, so we can run
 *            them round robin. Particularly, when the context switch occurs, we
 *            make current task as ready and push it to queue tail. And pop the
 *            next task from queue head, run it, and mark it as running.
 *          + The waiting queue contains processes which are in sleeping state.
 *            We push current process to this queue when it call Sleep() system
 *            call and pop them when Wakeup() is called.
 *          + The kill queue contains killed processes. We push killed processes
 *            to it, and the init process will pop and cleanup their resource.
 * 
 *          In this section, we are going to analyze each state of process.
 *          Let's get started.
 *
 *          1. Every process object is also maintained by `s_process_manager`
 *          (that actually is array of processes). So, if user does not request
 *          to create new process, we mark them as `PROCESS_SLOT_UNUSED`.
 *
 *          2. When user request to create a new process, we find a free process
 *          slot using `FindFreeProcessSlot()`, initialize the process object
 *          (stack, virtual memory map, context, trap frame, etc.) and mark it
 *          as `PROCESS_SLOT_INITIALIZED`.
 * 
 *          3. After that, we push it to the ready queue and mark it as
 *          `PROCESS_SLOT_READY` state.
 * 
 *          4. When context switch is run, and this is it's turn. The process is
 *          pop from the ready queue and mark as `PROCESS_SLOT_RUNNING`, it take
 *          CPU control from previous process. In the state, the process object
 *          not belong to any queue, this is maintain by
 *          scheduler->current_proc.
 * 
 *          5. When process is running state, if it doesn't call sleep() and
 *          exit(), and when the context switch occurs again, out of it's turn.
 *          The process will be push to ready list, gave CPU control to next
 *          process, and marked as `PROCESS_SLOT_READY`.
 *
 *          6. If a process is running state, and it call sleep() system call,
 *          we will push it to the while loop until the time is achieved. And in
 *          the while loop, we push it to the sleep queue, mark it as
 *          `PROCESS_SLOT_SLEEPING`, and pop it out of ready queue, so this
 *          process will never be run until we call the Wakeup(). Every 10ms
 *          the timer handler is called, we wakeup it, check the time is reached
 *          or not, if it is, we return the system call to user mode, if it is
 *          not, we sleep it again in the while loop().
 * 
 *          7. The `PROCESS_SLOT_KILLED` state is reached by three ways:
 *              + User call Exit() system call.
 *              + the user main() function return.
 *              + The process cause CPU generated a error exception (page fault, 
 *                segmentation fault, for example.)
 *          In this state we will not release process's resource intermediately,
 *          because, the process still running. So, we pop it out ready queue,
 *          and push it to the killed queue, by the way, the process will never
 *          be run again. And when init process wakeup, it call cleanup all
 *          resource of the process such as: kernel stack, virtual memory page
 *          map, etc. And finally, it marked the process as
 *          `PROCESS_SLOT_UNUSED`, to the next user create process request could
 *          reuse it's slot.
 * 
 * @version 0.1
 * @date 2023-08-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <stdint.h>
#include <list.h>
#include "common.h"
#include "trap.h"
#include "memory.h"

/* Public define -------------------------------------------------------------*/
#define STACK_SIZE                          PAGE_SIZE    /* 2MB. */
#define MAXIMUM_NUMBER_OF_PROCESS           10
#define USER_STACK_START                    (USER_VIRTUAL_ADDRESS_BASE \
                                            + STACK_SIZE)
#define NORMAL_PROCESS_WAIT_ID              -1
#define INIT_PROCESS_WAIT_ID                1
#define WAITING_KEYBOARD_PROCESS_WAIT_ID    -2
#define PROCESS_MAXIMUM_FILE_DESCRIPTOR     100
/* Public type ---------------------------------------------------------------*/
typedef enum  {
    PROCESS_SLOT_UNUSED = 0,
    PROCESS_SLOT_INITIALIZED,
    PROCESS_SLOT_READY,
    PROCESS_SLOT_RUNNING,
    PROCESS_SLOT_SLEEPING,
    PROCESS_SLOT_KILLED
} ProcessState;


/**
 * @brief   Process Control Block structure. This structure is used to store the
 *          essential data of the process. It is maintained in the kernel space,
 *          user program is not allowed to access it.
 *
 * @property next       - Next process to run.
 * @property pid        - Process Identification number of a process.
 * @property wait_id    - Save the process wait id.
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
struct FD;

typedef struct {
    List *next;
    int pid;
    int wait_id;
    ProcessState state;
    uint64_t page_map;
    uint64_t context;
    uint64_t stack;
    TrapFrame *tf;
    struct FD *file[PROCESS_MAXIMUM_FILE_DESCRIPTOR];
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

typedef struct {
    Process *current_proc;
    HeadList ready_proc_list;
    HeadList wait_proc_list;
    HeadList kill_proc_list;
} Scheduler;

/* Public function prototype -------------------------------------------------*/

void InitProcess(void);
void ProcessStart(TrapFrame *tf);
Scheduler *GetScheduler(void);

/**
 * @brief       Stop current process, mark it as ready state, context switch,
 *              and gave CPU control to next process to run.
 * 
 */
void Yield(void);

/**
 * @brief       Switch context between two process, save old context to `old`,
 *              And retrieve new context from `new`.
 * 
 * @param[in]   old   The address of context field in process.
 * @param[in]   new   The context value in the next process.
 * @return none
 */
void ContextSwitch(uint64_t *old, uint64_t new);

/**
 * @brief       Add current process to wait list wih wait_id.
 * 
 * @param[in]   wait_id     - Current process wait id.
 */
void Sleep(int wait_id);

/**
 * @brief       Wakeup processes which match wait_id.
 * 
 * @param[in]   wait_id     - Wait id to wake up processes matching.
 */
void Wakeup(int wait_id);

/**
 * @brief       Exit current process, remove it from ready list.
 * 
 */
void Exit(void);

/**
 * @brief       Waiting for children process exit, the init process call this to
 *              cleanup exiting processes. This system call never return from
 *              kernel mode, so, this function should be called when the init
 *              process has done it's job in user space.
 * 
 */
void Wait(int pid);

int Fork(void);

int Exec(Process *proc, const char *filename);