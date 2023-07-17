[BITS 64]
[ORG 0x200000]

Start:
    ; 1. Load GDT and load code segment descriptor to cs register.
    lgdt [GDT64Pointer]

    push 0x08           ; Push Code Selector.
    push KernelEntry    ; Push Kernel entry address.
    db 0x48             ; Set operand-size to 64 bit.
    retf                ; we load code segment descriptor by far return to
                        ; `caller` with caller address is KernelEntry.

KernelEntry:
    mov byte[0xB8000], 'K'
    mov byte[0xB8001], 0xA

KernelEnd:
    hlt
    jmp KernelEnd

; Global Descriptor Table Structure for 64 bit mode.
GDT64:
    dq 0            ; First entry is NULL.
CodeSegDes64:       ; Next entry is Code Segment Descriptor.
    dq 0x0020980000000000

GDT64Len: equ $-GDT64

GDT64Pointer: dw GDT64Len - 1   ; First two bytes is GDT length.
              dd GDT64          ; Next four bytes are GDT64 address.