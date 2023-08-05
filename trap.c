#include "trap.h"
#include "assert.h"
#include "printk.h"
#include "syscall.h"

/* Private define ------------------------------------------------------------*/
#define MAXIMUM_IRQ_NUMBER 256
#define KERNEL_CODE_SEGMENT_SELECTOR 0x08
/* Private variable ----------------------------------------------------------*/
static IDTPointer s_IDT_ptr;
static IDTEntry s_interrupt_entries[MAXIMUM_IRQ_NUMBER];

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief    This helper function initialize a interrupt descriptor table entry.
 *
 * @param[in] entry         - Interrupt descriptor table entry.
 * @param[in] address       - Handler base address.
 * @param[in] attribute     - Attribute.
 * @return    none
 */
static void InitIDTEntry(IDTEntry *entry, uint64_t address, uint8_t attribute);

/**
 * @brief    Interrupt handler.
 *
 * @param[in] tf            - The stack pointer that point to trap frame.
 * @return    none
 */
void InterruptHandler(TrapFrame *tf);

/* Public function -----------------------------------------------------------*/
void InitIDT(void)
{
    InitIDTEntry(&s_interrupt_entries[0], (uint64_t)Vector0, 0x8E);
    InitIDTEntry(&s_interrupt_entries[1], (uint64_t)Vector1, 0x8E);
    InitIDTEntry(&s_interrupt_entries[2], (uint64_t)Vector2, 0x8E);
    InitIDTEntry(&s_interrupt_entries[3], (uint64_t)Vector3, 0x8E);
    InitIDTEntry(&s_interrupt_entries[4], (uint64_t)Vector4, 0x8E);
    InitIDTEntry(&s_interrupt_entries[5], (uint64_t)Vector5, 0x8E);
    InitIDTEntry(&s_interrupt_entries[6], (uint64_t)Vector6, 0x8E);
    InitIDTEntry(&s_interrupt_entries[7], (uint64_t)Vector7, 0x8E);
    InitIDTEntry(&s_interrupt_entries[8], (uint64_t)Vector8, 0x8E);
    InitIDTEntry(&s_interrupt_entries[10], (uint64_t)Vector10, 0x8E);
    InitIDTEntry(&s_interrupt_entries[11], (uint64_t)Vector11, 0x8E);
    InitIDTEntry(&s_interrupt_entries[12], (uint64_t)Vector12, 0x8E);
    InitIDTEntry(&s_interrupt_entries[13], (uint64_t)Vector13, 0x8E);
    InitIDTEntry(&s_interrupt_entries[14], (uint64_t)Vector14, 0x8E);
    InitIDTEntry(&s_interrupt_entries[16], (uint64_t)Vector16, 0x8E);
    InitIDTEntry(&s_interrupt_entries[17], (uint64_t)Vector17, 0x8E);
    InitIDTEntry(&s_interrupt_entries[18], (uint64_t)Vector18, 0x8E);
    InitIDTEntry(&s_interrupt_entries[19], (uint64_t)Vector19, 0x8E);
    InitIDTEntry(&s_interrupt_entries[32], (uint64_t)Vector32, 0x8E);
    InitIDTEntry(&s_interrupt_entries[39], (uint64_t)Vector39, 0x8E);
    
    /* Init system call handler, DPL attribute is set to 3 instead of 0,
     * Because we will fire the interrupt in ring 3, so user can fire this
     * interrupt. */
    InitIDTEntry(&s_interrupt_entries[SYSTEM_CALL_INTERRUPT_NUMBER],
                (uint64_t)Syscall, 0xEE);

    s_IDT_ptr.limit = sizeof(s_interrupt_entries) - 1;
    s_IDT_ptr.address = (uint64_t)s_interrupt_entries;

    LoadIDT(&s_IDT_ptr);
}

/* Private function ----------------------------------------------------------*/
static void InitIDTEntry(IDTEntry *entry, uint64_t address, uint8_t attribute)
{
    entry->low = (uint16_t)address;
    entry->selector = KERNEL_CODE_SEGMENT_SELECTOR;
    entry->attr = attribute;
    entry->mid = (uint16_t)(address>>16);
    entry->high = (uint32_t)(address>>32);
}

void InterruptHandler(TrapFrame *tf)
{
    switch (tf->trapno) {
    case 32:        /* Timer interrupt. */
        EOI();
        break;
    
    case 39: {      /* Spurious interrupt. */
        uint8_t isr_value = ReadISR();
        if ((isr_value & (1<<7)) != 0) {
            EOI();
        }
    }
    break;
    case SYSTEM_CALL_INTERRUPT_NUMBER: {
        SystemCall(tf);
    }
    break;

    default: {
        char msg[70] = {0};
        sprintk(msg,
                "[Error %d at ring: %d] %d:%x %x",
                tf->trapno,         /* Trap number. */
                (tf->cs & 3),       /* Ring number. */
                tf->error_code,     /* Error code. */
                ReadCR2(),          /* Virtual address. */
                tf->rip);           /* Address of error instruction. */
        panic(msg);
    }

        break;
    }
}
