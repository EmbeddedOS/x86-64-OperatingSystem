
#include <stddef.h>
#include "syscall.h"
#include "assert.h"
#include "printk.h"

/* Private define ------------------------------------------------------------*/
#define MAXIMUM_SYSTEM_CALLS 20

/* Private variable ----------------------------------------------------------*/
static SYSTEM_CALL s_syscall_table[MAXIMUM_SYSTEM_CALLS] = {0};

/* Private function prototype ------------------------------------------------*/
static int SysWrite(int64_t *arg);
static void RegisterSystemCall(uint16_t num, SYSTEM_CALL call);

/* Public function -----------------------------------------------------------*/
void InitSystemCall(void)
{
    RegisterSystemCall(0, SysWrite);
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
