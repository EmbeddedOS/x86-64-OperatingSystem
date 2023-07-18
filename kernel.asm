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

[BITS 64]
[ORG 0x200000]

Start:
    ; 1. Setup Divide By 0 handler for that exception.
    mov rdi, IDT64
    mov rax, DivideBy0Handler
    mov [rdi], ax
    shr rax, 16
    mov [rdi + 6], ax
    shr rax, 16
    mov [rdi + 8], eax

    ; 2. Load GDT and load code segment descriptor to cs register.
    lgdt [GDT64Pointer]
    lidt [IDT64Pointer]

    push 0x08           ; Push Code Selector.
    push KernelEntry    ; Push Kernel entry address.
    db 0x48             ; Set operand-size to 64 bit.
    retf                ; we load code segment descriptor by far return to
                        ; `caller` with caller address is KernelEntry.

KernelEntry:
    mov byte[0xB8000], 'K'
    mov byte[0xB8001], 0xA
    xor rbx, rbx
    div rbx             ; Test divide by zero interrupt.

KernelEnd:
    hlt
    jmp KernelEnd

DivideBy0Handler:
    mov byte[0xB8002], 'D'
    mov byte[0xB8003], 0xA
    jmp KernelEnd
    iretq

; Global Descriptor Table Structure for 64 bit mode.
GDT64:
    dq 0            ; First entry is NULL.
CodeSegDes64:       ; Next entry is Code Segment Descriptor.
    dq 0x0020980000000000

GDT64Len: equ $-GDT64

GDT64Pointer: dw GDT64Len - 1   ; First two bytes is GDT length.
              dq GDT64          ; Next four bytes are GDT64 address.

; Interrupt Descriptor Table Structure.
IDT64:
    ; Repeat 256 times for all interrupt entries.
    %rep 256
    dw 0
    dw 0x08         ; Segment selector select code segment descriptor.
    db 0
    db 0b10001110   ; The attribute bit: P=1, DPL=0, TYPE=01110.
    dw 0
    dd 0
    dd 0
    %endrep

IDT64Len: equ $-IDT64

IDT64Pointer:   dw IDT64Len - 1
                dq IDT64
