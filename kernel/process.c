#include <errno.h>
#include <string.h>

#include "process.h"
#include "file.h"
#include "printk.h"
#include "assert.h"

/* Private Define ------------------------------------------------------------*/
#define IDLE_PROCESS_PID                0
#define USER_INIT_PROCESS_ADDRESS_BASE  0x20000         /* Our shell program. */
#define SIZE_OF_INIT_PROCCESS           (512 * 20)      /* 20 sectors.        */

/* Private variable ----------------------------------------------------------*/

extern TSS TaskStateSegment; /* Extern from ASM. */
static Process s_process_manager[MAXIMUM_NUMBER_OF_PROCESS];
static int s_pid_num = 1;
static Scheduler s_scheduler;

/* Private function prototypes -----------------------------------------------*/

static Process *FindFreeProcessSlot(void);
static Process *CreateNewProcess(void);
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

List *RemoveProcessWithPID(HeadList *list, int pid);

/**
 * @brief   This function initialize the IDLE task. If the ready process list is
 *          empty, we run IDLE task. And the IDLE do nothing, just jump loop in
 *          KernelEnd, and enable interrupt, waiting for new context switch
 *          event, triggered by the timer interrupt.
 *          IDLE task always have PID 0, and this function should be call first.
 */ 
static void InitIDLEProcess(void);

static void InitShellProcess(void);

/* Public function -----------------------------------------------------------*/
void InitProcess(void)
{
    /* Init IDLE process first. */
    InitIDLEProcess();

    /* Run INIT process (Shell). */
    InitShellProcess();
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

    /* We don't push the IDLE task to the ready list. */
    if (proc->pid != IDLE_PROCESS_PID) {
        ListPushBack(list, (List *)proc);
    }

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

void Wait(int pid)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->kill_proc_list;

    /* Forever loop to cleanup resources of all killed process. */
    while (1) {
        if (!ListIsEmpty(list)) {
            proc = (Process *)RemoveProcessWithPID(list, pid);
            if (proc != NULL) {
                ASSERT(proc->state == PROCESS_SLOT_KILLED);

                /* Cleanup the process. */
                kfree(proc->stack);
                FreeVM(proc->page_map);
                
                /* Close opened files. */
                for (int i = USER_START_FD;
                     i < PROCESS_MAXIMUM_FILE_DESCRIPTOR;
                     i++) {

                    if (proc->file[i] != NULL) {
                        proc->file[i]->fcb->open_count--;
                        proc->file[i]->open_count--;

                        if (proc->file[i]->open_count == 0) {
                            proc->file[i]->fcb = NULL;
                        }
                    }
                }

                memset(proc, 0, sizeof(Process));
                break;
            }
        } else {
            /* Sleep if we don't have any killed process. */
            Sleep(INIT_PROCESS_WAIT_ID);
        }
    }
}

int Fork(void)
{
    Process *proc = NULL;
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->ready_proc_list;
    Process *current_proc = scheduler->current_proc;

    proc = CreateNewProcess();
    if (proc == NULL) {
        printk("DEBUG: Failed to create new process.\n");
        return -ENOMEM;
    }

    if (!CopyUVM(proc->page_map, current_proc->page_map, PAGE_SIZE)) {
        printk("DEBUG: Failed to copy virtual memory.\n");
        return -ENOMEM;
    }

    /* Copy FD table, so the new process will point to same FD entries. */
    memcpy(proc->file,
           current_proc->file,
           sizeof(FD *) * PROCESS_MAXIMUM_FILE_DESCRIPTOR);

    /* Handling shared files. */
    for (int i = USER_START_FD; i < PROCESS_MAXIMUM_FILE_DESCRIPTOR; i++)
    {
        if (proc->file[i] != NULL) {
            /* We increase counters, means the new process will use them also. */
            proc->file[i]->fcb->open_count++;
            proc->file[i]->open_count++;
        }
    }

    /* Copy the trap frame, therefore the new process will return to the same
     * location as the current process does. */
    memcpy(proc->tf, current_proc->tf, sizeof(TrapFrame));

    /* This is return value in new process when it back to user mode. */
    proc->tf->rax = 0;

    /* Append it to ready list. */
    proc->state = PROCESS_SLOT_READY;
    ListPushBack(list, (List *)proc);

    /* For current process, we return pid of new process. */
    return proc->pid;
}

int Exec(Process *proc, const char *filename)
{
    int fd = 0;
    int program_size = 0;

    fd = Open(proc, filename);
    if (fd < 0) {
        /* If we cannot open the file, we exit current process. */
        printk("DEBUG: Cannot open file.\n");
        Exit();
    }

    /* Clear all virtual memory. */
    memset((void *)USER_VIRTUAL_ADDRESS_BASE, 0, PAGE_SIZE);
    program_size = GetFileSize(proc, fd);

    /* Copy all program file to virtual address base. */
    program_size = Read(proc,
                        fd,
                        (void *)USER_VIRTUAL_ADDRESS_BASE,
                        program_size);

    if (program_size < 0) {
        /* Exit if we can not read data file. */
        Exit();
    }

    Close(proc, fd);

    /* Clear trap frame and set it to default mode. */
    memset(proc->tf, 0, sizeof(TrapFrame));
    proc->tf->cs = 0x10 | 3;
    proc->tf->rip = USER_VIRTUAL_ADDRESS_BASE;
    proc->tf->ss = 0x18 | 3;
    proc->tf->rsp = USER_STACK_START;
    proc->tf->rflags = 0x202;

    return 0;
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
    prev_proc = scheduler->current_proc;

    if (ListIsEmpty(list)) {
        /* If the ready list is empty we run IDLE task next. */
        current_proc = &s_process_manager[IDLE_PROCESS_PID];
    } else {
        current_proc = (Process *)ListPopFront(list);
    }

    /* Get head ready process and make it as running. */
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

List *RemoveProcessWithPID(HeadList *list, int pid)
{
  List *current = list->next;
    List *prev = (List *)list;
    List *item = NULL;

    while(current !=NULL) {
        /* remove from list if matching pid. */
        if (((Process *)current)->pid == pid) {
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


static void InitIDLEProcess(void)
{
    Process *proc;
    proc = FindFreeProcessSlot();
    proc->pid = IDLE_PROCESS_PID;
    proc->page_map = PHY_TO_VIR(ReadCR3());
    proc->state = PROCESS_SLOT_RUNNING;
    GetScheduler()->current_proc = proc;
}

static void InitShellProcess(void)
{
    Scheduler *scheduler = GetScheduler();
    HeadList *list = &scheduler->ready_proc_list;

    Process *proc = CreateNewProcess();

    /* Map user memory (2MB) to the kernel virtual memory we just made. */
    ASSERT(SetupUVM(proc->page_map,
            (uint64_t)PHY_TO_VIR(USER_INIT_PROCESS_ADDRESS_BASE),
            SIZE_OF_INIT_PROCCESS));

    proc->state = PROCESS_SLOT_READY;
    ListPushBack(list, (List *)proc);
}

Process* CreateNewProcess(void)
{
    uint64_t stack_top = 0;
    Process * proc = FindFreeProcessSlot();
    if (proc == NULL) {
        return NULL;
    }

    /* Each process has 2MB its own kernel stack. */
    proc->stack = (uint64_t)kalloc();
    if (proc->stack == 0) {
        return NULL;
    }

    proc->state = PROCESS_SLOT_INITIALIZED;
    proc->pid = s_pid_num++;
    proc->wait_id = 0;

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
    if (proc->page_map == 0) {
        kfree(proc->stack);
        memset(proc, 0, sizeof(Process));
        return NULL;
    }

    return proc;
}
