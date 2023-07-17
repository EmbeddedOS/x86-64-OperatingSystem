[BITS 64]
[ORG 0x200000]

Start:
    mov byte[0xB8000], 'K'
    mov byte[0xB8001], 0xA

KernelEnd:
    hlt
    jmp KernelEnd