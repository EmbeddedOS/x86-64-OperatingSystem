#include "process.h"
#include "printk.h"
#include "assert.h"
#include <string.h>

/* Private variable ----------------------------------------------------------*/

extern TSS TaskStateSegment; /* Extern from ASM. */
static Process s_process_manager[MAXIMUM_NUMBER_OF_PROCESS];
static int s_pid_num = 1;
static Scheduler s_scheduler;

/* Private function prototypes -----------------------------------------------*/

static Process *FindFreeProcessSlot(void);
static void SetProcessEntry(Process* proc, uint64_t user_addr);

/**
 * @brief   Set TaskStateSegment point to top of the process's kernel stack. So
 *          when we jump from ring 3 to ring 0, the kernel stack will be used.
 * 
 * @param   proc 
 * @return  none
 */
static void SetTSS(Process *proc);

static void Schedule(void);

static void SwitchProcess(Process *prev, Process *new);

List *WaitListRemoveReadyProcess(HeadList *list, int wait_id);

/* Public function -----------------------------------------------------------*/
void InitProcess(void)
{
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->ready_proc_list;

    uint64_t addr[3] = {0x20000, 0x30000, 0x40000};

    for (int i = 0; i < 3; i++) {
        Process *proc = FindFreeProcessSlot();
        SetProcessEntry(proc, addr[i]);
        ListPushBack(list, (List *)proc);
    }
}

void StartScheduler(void)
{
    Scheduler *scheduler = GetScheduler();

    /* 1. Get process from ready list. */
    Process *proc = (Process *)ListPopFront(&scheduler->ready_proc_list);

    /* 2. Make process as running. */
    proc->state = PROCESS_SLOT_RUNNING;
    scheduler->current_proc = proc;

    /* 3. Set TaskStateSegment point to it's kernel stack. */
    SetTSS(proc);

    /* 4. Switch to process virtual memory. */
    SwitchVM(proc->page_map);

    /* 5. Start process program. */
    ProcessStart(proc->tf);
}

Scheduler *GetScheduler(void)
{
    return &s_scheduler;
}

void Yield(void)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->ready_proc_list;

    if (ListIsEmpty(list)) {
        return;
    }

    /* Get current process, set it as ready, and push it to back of the ready
     * list. */
    proc = scheduler->current_proc;
    proc->state = PROCESS_SLOT_READY;
    ListPushBack(list, (List *)proc);

    /* Process switch. */
    Schedule();
}

void Sleep(int wait_id)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->wait_proc_list;

    /* Push current process to sleep process list. */
    proc = scheduler->current_proc;
    proc->state = PROCESS_SLOT_SLEEPING;
    proc->wait_id = wait_id;

    ListPushBack(list, (List *)proc);

    /* Re-schedule to run next process. */
    Schedule();
}

void Wakeup(int wait_id)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *wait_list = &scheduler->wait_proc_list;
    HeadList *ready_list = &scheduler->ready_proc_list;

    /* Find correct processes which are wake up time and remove it from wait
     * list. */
    proc = (Process *)WaitListRemoveReadyProcess(wait_list, wait_id);

    while (proc != NULL)
    {
        /* Push the process to ready list if now is it's wakeup time. */
        proc->state = PROCESS_SLOT_READY;
        ListPushBack(ready_list, (List *)proc);

        /* Check another processes in the wait list. */
        proc = (Process *)WaitListRemoveReadyProcess(wait_list, wait_id);
    }
}

void Exit(void)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->kill_proc_list;

    proc = scheduler->current_proc;
    proc->state = PROCESS_SLOT_KILLED;

    ListPushBack(list, (List *)proc);

    /* We need to call Wakeup() function because the init process which is first
     * process in this system is constantly calling Wait() function to do the
     * cleaning up, and the process maybe is sleeping state. So we wake it up
     * before we switch to another process.
     */
    Wakeup(INIT_PROCESS_WAIT_ID);

    /* We re-schedule, the current process will be pop from ready list. */
    Schedule();
}

void Wait(void)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->kill_proc_list;

    /* Forever loop to cleanup resources of all killed process. */
    while (1) {
        if (!ListIsEmpty(list)) {
            proc = (Process *)ListPopFront(list);
            ASSERT(proc->state == PROCESS_SLOT_KILLED);

            /* Cleanup the process. */
            kfree(proc->stack);
            FreeVM(proc->page_map);
            memset(proc, 0, sizeof(Process));
            proc->state = PROCESS_SLOT_UNUSED;
        } else {
            /* Sleep if we don't have any killed process. */
            Sleep(INIT_PROCESS_WAIT_ID);
        }
    }
}

/* Private function ----------------------------------------------------------*/
static Process *FindFreeProcessSlot(void)
{
    Process *proc = NULL;

    for (int i = 0; i < MAXIMUM_NUMBER_OF_PROCESS; i++)
    {
        if (s_process_manager[i].state == PROCESS_SLOT_UNUSED) {
            proc = &s_process_manager[i];
            break;
        }
    }

    return proc;
}

static void SetProcessEntry(Process* proc, uint64_t user_addr)
{
    uint64_t stack_top;

    proc->state = PROCESS_SLOT_INITIALIZED;
    proc->pid = s_pid_num++;
    proc->wait_id = 0;

    /* Each process has 2MB its own kernel stack. */
    proc->stack = (uint64_t)kalloc();
    ASSERT(proc->stack != 0);

    memset((void *)proc->stack, 0, STACK_SIZE);
    stack_top = proc->stack + STACK_SIZE;

    /* Because the process is not run until now, so it don't have the context.
     * We make a empty context to it. That include 6 context registers, and
     * return address. So, we make context point to `rsp` - 7 * 8. */
    proc->context = stack_top - sizeof(TrapFrame) - 7*8;
    /* We save the address of `TrapReturn` to rsp + 6. The next value of stack
     * pointer will be return address. So we push `TrapReturn` to it. */
    *(uint64_t *)(proc->context + 6*8) = (uint64_t)TrapReturn;

    /* We save stack frame at top of kernel stack. */
    proc->tf = (TrapFrame *)(stack_top - sizeof(TrapFrame));

    proc->tf->cs = 0x10 | 3;
    proc->tf->rip = USER_VIRTUAL_ADDRESS_BASE;
    proc->tf->ss = 0x18 | 3;
    proc->tf->rsp = USER_STACK_START;
    proc->tf->rflags = 0x202;

    /* We create a virtual memory that is mapped with kernel, so the kernel will
     * reside at the same address in every user virtual memory. */
    proc->page_map = SetupKVM();
    ASSERT(proc->page_map != 0);

    /* Map user memory (2MB) to the kernel virtual memory we just made. */
    ASSERT(SetupUVM(proc->page_map, (uint64_t)PHY_TO_VIR(user_addr), 512 * 10));

    proc->state = PROCESS_SLOT_READY;
}

static void SetTSS(Process *proc)
{
    /* We set TSS structure by assigning the top of the kernel stack to rsp0 in
     * the TaskStateSegment. */
    TaskStateSegment.rsp0 = proc->stack + STACK_SIZE;
}

static void Schedule(void)
{
    Process *prev_proc = NULL;
    Process *current_proc = NULL;

    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->ready_proc_list;
    ASSERT(!ListIsEmpty(list));

    prev_proc = scheduler->current_proc;

    /* Get head ready process and make it as running. */
    current_proc = (Process *)ListPopFront(list);
    current_proc->state = PROCESS_SLOT_RUNNING;
    scheduler->current_proc = current_proc;

    /* Switch to new process. */
    SwitchProcess(prev_proc, current_proc);
}

static void SwitchProcess(Process *prev, Process *new)
{
    SetTSS(new);
    SwitchVM(new->page_map);
    ContextSwitch(&prev->context, new->context);
}

List *WaitListRemoveReadyProcess(HeadList *list, int wait_id)
{
    List *current = list->next;
    List *prev = (List *)list;
    List *item = NULL;

    while(current !=NULL) {
        /* Wakeup if matching id. */
        if (((Process *)current)->wait_id == wait_id) {
            /* If found a ready process return it. */
            prev->next = current->next;
            item = current;

            if (list->next == NULL) {
                /* Current list is empty. */
                list->tail = NULL;
            } else if (current->next == NULL) {
                /* The last one item in the list. */
                list->tail = prev;
            }

            break;
        }

        /* Check next process in the list. */
        prev = current;
        current = current->next;
    }

    return item;
}
