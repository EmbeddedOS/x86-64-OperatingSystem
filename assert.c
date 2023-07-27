#include "assert.h"
#include "printk.h"

#define ASSERTION_FAILURE_MESSAGE_MAX_SIZE 100

void error_point(const char *file, uint64_t line)
{
    char msg[ASSERTION_FAILURE_MESSAGE_MAX_SIZE] = {0}; 
    sprintk(msg, "Assertion failed at %s:%d", file, line);
    panic(msg);
}


void panic(const char *msg)
{
    /* TODO: print stack trace here. */
    printk("KERNEL PANIC: %s\n", msg);
    while (1);
}