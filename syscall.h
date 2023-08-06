/**
 * @file    syscall.h
 * @author  Cong Nguyen (congnt264@gmail.com)
 * @brief   System calls are used to call a kernel server from user land. The
 *          goal is to able to switch from user mode to kernel mode, with the
 *          privileges. The most common way to implement system calls is using a
 *          software interrupt. It is probably the most portable way to
 *          implement system calls. Linux traditionally uses interrupt 0x80 for
 *          this purpose on x86. So, for our system, we also do it and design
 *          calling conventions by ourself (we still use x86 calling convention,
 *          but it does not cover at all, so we design new conventions to make
 *          our system call system).
 *          Before we start, we need to note that, in our system, every user
 *          virtual memory reference to kernel code at the same address
 *          KERNEL_VIRTUAL_ADDRESS_BASE, so we don't need to switch virtual
 *          memory when switch between user mode and kernel mode.
 * 
 *          Let's get started.
 * 
 *          Step 1: User prepare for requesting service:
 *              + When the user program want to request a kernel service, it
 *                have to prepare argument by itself.
 *              + Step 1.1: Create a stack frame (number of arguments * 8) bytes
 *                to save arguments.
 *              + Step 1.2: Set system call number to `eax` register.
 *              + Step 1.3: Set arguments to the stack frame.
 *              + Step 1.4: set number of arguments to `rdi`.
 *              + Step 1.5: Set stack pointer (User stack frame) to `rsi`.
 *              + Step 1.6: Calling interrupt 0x80, so from now we run in
 *                kernel mode.
 *          Step 2: Kernel catch the interrupt and handle it.
 *              + Normally, all interrupts are only fired in ring 0, so for 0x80
 *                we have to set it's attribute to able to fire it in user code
 *                (ring 3).
 *              + Step 2.1: When CPU jump to interrupt handle `Syscall`, we push
 *                `error_code` as 0 and `trapno` as 0x80 to stack and jump to
 *                `Trap`.
 *              + Step 2.2: In the trap, we save CPU states (Note that here, the
 *                CPU actually save `rip`, `cs`, `rflags`, `rsp`, `ss`
 *                automatically, but these are not all CPU state, we
 *                additionally save `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`,
 *                `rbp`, `r8`, `r9`, `r10`, `r11`, `r12`, `r13`, `r14`, `r15`)
 *                to stack.
 *              + Step 2.3: We set stack pointer to `rdi` and call
 *                `InterruptHandler` in C, so by x86 calling conventions, `rdi`
 *                will by pass to first argument, by this way, we get stack
 *                frame (Trap frame).
 *                In kernel code C, we define Trap frame struct that mapping to
 *                Trap frame which is push to stack, this structure
 *                actually contain:
 *                  + Registers are pushed automatically by processor.
 *                  + `error_code` and `trapno` we push in `Syscall`.
 *                  + General purpose registers that hold the CPU state which we
 *                    additionally pushed.
 *                In `InterruptHandler` we get trap number 0x80, and call
 *                `SystemCall` to handle user request.
 *              + Step 2.4: Check user request is valid and perform service:
 *                  + So when we have stack frame, we can get arguments which
 *                    user set before they call `int 0x80` (step 1).
 *                  + We get system call number from trap frame -> `rax`.
 *                  + We get param count via trap frame -> `rdi`.
 *                  + we get params via trap frame -> `rsi` that is the user
 *                    stack frame address.
 *                  + Calling corresponding service.
 *                  + Set return code to trap frame -> `rax`.
 *                  + Return to the `Trap`.
 *          Step 3: Return to the trap, restore CPU state and back to user mode.
 *              + After return from interrupt handler in C, we back to the trap.
 *              + Step 3.1: Restore all register (pop from stack) that we saved
 *                additionally in step 2.3. These are:
 *                  + `error_code` and `trapno` we push in `Syscall`.
 *                  + General purpose registers that hold the CPU state which we
 *                    additionally pushed.
 *              + Note here, the `rax` is restore also, but we changed it to
 *                send error code back to user.
 *              + Call `iretq` to return from IRQ, so from now we run in user
 *                mode.
 *          Step 4: After back from kernel, we clear user stack frame, we make
 *          in Step 1.1.
 *              + The `rax` still hold return from kernel, so if we call this
 *                function in C, this will be returned automatically by x86
 *                calling conventions.
 *
 *          So that is the way we request a kernel service from user, the kernel
 *          service actually run in interrupt 0x80 context.
 *          And also note that when we switch to kernel mode, the stack we use
 *          actually is the process's kernel stack.
 * 
 * 
 * @version 0.1
 * @date 2023-08-05
 * 
 * @copyright Copyright (c) 2023
 * 
 */
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