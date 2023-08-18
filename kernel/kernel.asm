; The first thing we will do in the kernel is manage the system resources in one
; place such as GDT and IDT. For GDT, we need to reload GDT and load code
; segment descriptor to cs register. Instead using jump instruction, we load
; descriptor using the far return instruction. When we execute the return
; instruction, we know that we will return from current procedure to the caller.
; The return address is pushed on the stack by the caller and go to procedure.
; In the procedure, return instruction will pop the address in `rip` register
; which will branch to that location. WHat we are actually doing here is
; fabricate the scenario where we are called by other caller and the far return
; instruction executes and we are back to the caller. So the return address need
; to be pushed on the stack on our own and we will jump to that address.

; we don't use the `ORG` directive to tell the assembler we want our kernel file
; running in the address 0x200000. But in the loader file, we still jump to the
; address 0x200000 after we load the kernel. So we use linker script to do it.

section .data
global TaskStateSegment

; Global Descriptor Table Structure for 64 bit mode.
GDT64:
    dq 0            ; First entry is NULL.
CodeSegDes64:       ; Next entry is Code Segment Descriptor.
    dq 0x0020980000000000
CodeSegDes64Ring3:
    dq 0x0020F80000000000   ; DPL is ring 3, we make new code segment descriptor
                            ; that run with privilege level 3.
DataSegDes64Ring3:
    dq 0x0000F20000000000   ; And make data segment descriptor that run with
                            ; privilege level 3 also and writable.
TaskStateSegDes64:          ; Task state segment descriptor.
    dw TssLen - 1           ; First two bytes are the lower 16 bits of TSS limit
    dw 0                    ; Lower 24 bits of base address is set to 0.
    db 0
    db 0b10001001           ; Attribute byte: P=1, DPL=00,TYPE=010001
    db 0
    db 0
    dq 0

GDT64Len: equ $-GDT64

GDT64Pointer: dw GDT64Len - 1   ; First two bytes is GDT length.
              dq GDT64          ; Next four bytes are GDT64 address.

; Task state segment structure - 128 bytes.
TaskStateSegment:
    dd 0                    ; First four bytes is reserved.
    dq 0xFFFF800000190000   ; Set RSP0 to new address.
    times 88 db 0           ; Clear next 88 bytes to 0.
    dd TssLen               ; IO permission bitmap, assign size of TSS means we
                            ; don't use the IO permission bitmap.
TssLen: equ $-TaskStateSegment

section .text
extern KMain

global Start        ; Declare the start of the kernel globally so that linker
                    ; will find it.

Start:
    ; 1. Load GDT and IDT.
    mov rax, GDT64Pointer   ; We need to move to rax first, because the kernel
    lgdt [rax]              ; now is higher memory.

    ; 2. Set Task state segment.
SetTSS:
    mov rax, TaskStateSegment       ; Point to TSS structure.
    mov rdi, TaskStateSegDes64
    mov [rdi + 2], ax
    shr rax, 16
    mov [rdi + 4], al
    shr rax, 8
    mov [rdi + 7], al
    shr rax, 8
    mov [rdi + 8], eax
    mov ax, 0x20                    ; Tss des is 5th entry, so we move 0x20.
    ltr ax                          ; Load TSS.

    ; 3. Initialize PIT - Programable Interval Timer.
InitializePIT:
    mov al, 0b00110100  ; Initialize PIT mode command register, FORM=0, MODE=010
                        ; , ACCESS=11, CHANNEL=00.
    out 0x43, al        ; Address of mode command register is 0x43. We use out
                        ; instruction to write the value in al to the register.
    mov ax, 11931       ; The interval we want to set to fire the interrupt each
                        ; 100Hz (CPU count with 1.2Mega Hz): 1193182/100 = 11931
    out 0x40, al        ; Address of data register channel 0 is 0x40. To set
    mov al, ah          ; interval, we out lower byte first and out higher byte
    out 0x40, al        ; after that.

    ; 4. Initialize PIC - Programable Interrupt Controller.
InitializePIC:
    mov al, 0b00010001  ; Initialize PIC command register bits[7:4]=0001,
                        ; bits[3:0]=0001.
    out 0x20, al        ; Write to the command register of master chip.
    out 0xA0, al        ; Write to the command register of master chip.

    mov al, 32          ; Write to the data register of master chip, set first
    out 0x21, al        ; number in vectors we want to use, we will 32->39 for
                        ; master chip.
    mov al, 40          ; And 40->47 for slave chip.
    out 0xA1, al

    mov al, 0b00000100  ; Select bit 2 in master chip to connect with slave.
    out 0x21, al
    mov al, 0b00000010  ; Slave identification should be 2.
    out 0xA1, al

    mov al, 0b00000001  ; Selecting mode: bit[0]=1 means x86 system is used.
    out 0x21, al
    out 0xA1, al

    mov al, 0b11111100  ; Masking all interrupts, except IRQ0 for the timer
    out 0x21, al        ; interrupt we used and IRQ1 for keyboard.
    mov al, 0b11111111
    out 0xA1, al

    ; 5. Load code segment descriptor to cs register.
    push 0x08           ; Push Code Selector.
    mov rax, KernelEntry
    push rax            ; Push Kernel entry address.
    db 0x48             ; Set operand-size to 64 bit.
    retf                ; we load code segment descriptor by far return to
                        ; `caller` with caller address is KernelEntry.

    ; 6. Jump to kernel main.
KernelEntry:
    xor ax, ax
    mov ss, ax

    mov rsp, 0xFFFF800000200000   ; Adjust kernel stack pointer that is new
                                  ; stack pointer we will use in the C code.
    call KMain

    ; If no tasks to run, the kernel go to here, we still enable interrupt for
    ; IDLE task.
KernelEnd:
    sti
    hlt
    jmp KernelEnd