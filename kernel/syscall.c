
#include <stddef.h>
#include "file.h"
#include "process.h"
#include "keyboard.h"
#include "syscall.h"
#include "memory.h"
#include "assert.h"
#include "printk.h"

/* Private define ------------------------------------------------------------*/
#define MAXIMUM_SYSTEM_CALLS 20

/* Private variable ----------------------------------------------------------*/
static SYSTEM_CALL s_syscall_table[MAXIMUM_SYSTEM_CALLS] = {0};

/* Private function prototype ------------------------------------------------*/
static int SysWrite(int64_t *arg);
static int SysSleep(int64_t *arg);
static int SysExit(int64_t *arg);
static int SysWait(int64_t *arg);
static int SysRead(int64_t *arg);
static int SysOpen(int64_t *arg);
static int SysClose(int64_t *arg);

static int SysMemInfo(int64_t *arg);

static void RegisterSystemCall(uint16_t num, SYSTEM_CALL call);

/* Public function -----------------------------------------------------------*/
void InitSystemCall(void)
{
    RegisterSystemCall(0, SysWrite);
    RegisterSystemCall(1, SysSleep);
    RegisterSystemCall(2, SysExit);
    RegisterSystemCall(3, SysWait);
    RegisterSystemCall(4, SysRead);
    RegisterSystemCall(5, SysMemInfo);
    RegisterSystemCall(6, SysOpen);
    RegisterSystemCall(7, SysClose);

}

void SystemCall(TrapFrame *tf)
{
    /* `rax` is used to hold the index number of system call.
     * `rdi` hold the param count.
     * `rsi` points to the arguments we passed to the function.
     * We return to the ring 3 user application via `rax`. */
    int64_t syscall_number = tf->rax;
    int64_t param_count = tf->rdi;
    int64_t *arg = (int64_t *)tf->rsi;

    if (param_count < 0
        || syscall_number > MAXIMUM_SYSTEM_CALLS) {
        tf->rax = -1;
        return;
    }
    ASSERT(s_syscall_table[syscall_number] != NULL);
    tf->rax = s_syscall_table[syscall_number](arg);
}

/* Private function ----------------------------------------------------------*/
static void RegisterSystemCall(uint16_t num, SYSTEM_CALL call)
{
    ASSERT(num < MAXIMUM_SYSTEM_CALLS);
    ASSERT(s_syscall_table[num] == NULL);
    s_syscall_table[num] = call;
}

static int SysWrite(int64_t *arg)
{
    /* TODO: implement file descriptor manager. */
    int16_t file_descriptor = arg[0];
    char *buffer = (char *)arg[1];
    int32_t length = arg[2];
    printk(buffer);
    // printk("User send: %d, %s, %d\n", file_descriptor, buffer, length);
    return length;
}

static int SysSleep(int64_t *arg)
{
    uint64_t old_ticks = 0;
    uint64_t ticks = 0;
    uint64_t sleep_ticks = arg[0];

    /* Get current ticks. */
    ticks = GetTicks();
    old_ticks = ticks;

    /* Block current process here until wakeup time is retrieved. */
    while (ticks - old_ticks < sleep_ticks) {
        Sleep(NORMAL_PROCESS_WAIT_ID);
        /* After wakeup from sleep, we get ticks again, and check process is
         * ready to run or not. If the time is not achieved, we sleep again. */
        ticks = GetTicks();
    }

    return 0;
}

static int SysExit(int64_t *arg)
{
    Exit();
    return 0;
}

static int SysWait(int64_t *arg)
{
    Wait();
    return 0;
}

static int SysRead(int64_t *arg)
{
    int16_t file_descriptor = arg[0];

    char *buffer = (char *)arg[1];
    int32_t length = arg[2];
    if (file_descriptor == STANDARD_INPUT) {
        /* Read from standard input. */
        for (int i = 0; i < length; i++) {
            buffer[i] = ReadKeyBuffer();
        }

        return length;
    }

    /* Read file. */
    return Read(GetScheduler()->current_proc, file_descriptor, buffer, length);
}

static int SysMemInfo(int64_t *arg)
{
    return GetTotalMem();
}

static int SysOpen(int64_t *arg)
{
    char *file_name = arg[0];
    return Open(GetScheduler()->current_proc, file_name);
}

static int SysClose(int64_t *arg)
{
    int16_t file_descriptor = arg[0];
    Close(GetScheduler()->current_proc, file_descriptor);
    return 0;
}