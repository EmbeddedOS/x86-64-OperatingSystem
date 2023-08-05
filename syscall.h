#pragma once
#include "trap.h"

/* Public type ---------------------------------------------------------------*/
/**
 * @brief       - System call handlers.
 * 
 * @param arg   - Data on the stack in user mode.
 * @return      - Error code to return to the user.
 */
typedef int (*SYSTEM_CALL)(int64_t *arg);

/* Public function prototype -------------------------------------------------*/
void InitSystemCall(void);
void SystemCall(TrapFrame *tf);