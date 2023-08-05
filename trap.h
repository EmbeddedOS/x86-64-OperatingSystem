#pragma once

#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define SYSTEM_CALL_INTERRUPT_NUMBER    0x80

/* Public type ---------------------------------------------------------------*/

typedef struct {
    uint16_t low;           /* Lower 16 bits of offset.     */
    uint16_t selector;      /* Selector.                    */
    uint8_t res0;           /* Reserved.                    */
    uint8_t attr;           /* Attributes.                  */
    uint16_t mid;           /* Mid 16 bits of offset.       */
    uint32_t high;          /* Higher 32 bits of offset.    */
    uint32_t res1;          /* Reserved.                    */
} IDTEntry;

typedef struct {
    uint16_t limit;
    uint64_t address;
} __attribute__ ((packed)) IDTPointer;

typedef struct {

    /* General purpose registers that hold the CPU state. */
    int64_t r15;
    int64_t r14;
    int64_t r13;
    int64_t r12;
    int64_t r11;
    int64_t r10;
    int64_t r9;
    int64_t r8;
    int64_t rbp;
    int64_t rdi;
    int64_t rsi;
    int64_t rdx;
    int64_t rcx;
    int64_t rbx;
    int64_t rax;

    /* Index number and some of the error code are pushed manually in asm. */
    int64_t trapno;
    int64_t error_code;

    /* Registers are pushed automatically by processor. */
    int64_t rip;
    int64_t cs;
    int64_t rflags;
    int64_t rsp;
    int64_t ss;
} TrapFrame;

/* Public function prototype -------------------------------------------------*/

/**
 * @brief    This function initialize interrupt descriptor table.
 *
 * @param[in] none
 * @return    none
 */
void InitIDT(void);

void Vector0(void);
void Vector1(void);
void Vector2(void);
void Vector3(void);
void Vector4(void);
void Vector5(void);
void Vector6(void);
void Vector7(void);
void Vector8(void);
void Vector10(void);
void Vector11(void);
void Vector12(void);
void Vector13(void);
void Vector14(void);
void Vector16(void);
void Vector17(void);
void Vector18(void);
void Vector19(void);
void Vector32(void);
void Vector39(void);
void Syscall(void);
void EOI(void);
uint8_t ReadISR(void);
void LoadIDT(IDTPointer *ptr);
uint64_t ReadCR2(void);