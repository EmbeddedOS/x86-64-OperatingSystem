section .text

extern InterruptHandler

; Interrupt handler WRAPPER, some vector numbers are reserved (9, 15, etc). 
global Vector0      ; Divide by zero.
global Vector1
global Vector2
global Vector3
global Vector4
global Vector5
global Vector6
global Vector7
global Vector8
global Vector10
global Vector11
global Vector12
global Vector13
global Vector14
global Vector16
global Vector17
global Vector18
global Vector19
global Vector32
global Vector39

global EOI          ; The end of interrupt.
global ReadISR
global LoadIDT
global LoadCR3
global ReadCR2
global ProcessStart

Trap:               ; Trap procedure: Save the CPU state by pushing the general
    push rax        ; purpose registers. Print character to debug. And call the
    push rbx        ; InterruptHandler in C and pass the first argument to the 
    push rcx        ; RDI register (Follow system V AMD64 calling convention).
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp    ; Pass stack pointer to the InterruptHandler.
    call InterruptHandler 
TrapReturn:         ; When InterruptHandler return, we back to the trap, and
    pop r15         ; restore state of the CPU.
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 0x10   ; Before we return, we need to adjust RSP register to make
    iretq           ; it point to correct location. Because we push 128 bytes
                    ; before jump to trap. We add 2 bytes to make the RSP point
                    ; to the original location when the exception or interrupt
                    ; gets called by the processor. If the error code is pushed
                    ; by processor (for example Vector8), we still need add 8
                    ; bytes manually.

Vector0:
    push 0          ; Error code, we need to push it in the system exception.
                    ; CPU don't make it.
    push 0          ; Index.
    jmp Trap

Vector1:
    push 0
    push 1
    jmp Trap

Vector2:
    push 0
    push 2
    jmp Trap

Vector3:
    push 0
    push 3
    jmp Trap

Vector4:
    push 0
    push 4
    jmp Trap

Vector5:
    push 0
    push 5
    jmp Trap

Vector6:
    push 0
    push 6
    jmp Trap

Vector7:
    push 0
    push 7
    jmp Trap

Vector8:            ; Error code of Interrupts are push automatically by CPU.
    push 8          ; So, we only need push the index.
    jmp Trap

Vector10:
    push 10
    jmp Trap

Vector11:
    push 11
    jmp Trap

Vector12:
    push 12
    jmp Trap

Vector13:
    push 13
    jmp Trap

Vector14:
    push 14
    jmp Trap

Vector16:
    push 0
    push 16
    jmp Trap

Vector17:
    push 17
    jmp Trap

Vector18:
    push 0
    push 18
    jmp Trap

Vector19:
    push 0
    push 19
    jmp Trap

Vector32:
    push 0
    push 32
    jmp Trap

Vector39:
    push 0
    push 39
    jmp Trap

EOI:                ; Send EOI to PIC.
    mov al, 0x20
    out 0x20, al
    ret

ReadISR:
    mov al, 0xB
    out 0x20, al
    in al, 0x20
    ret

LoadIDT:
    lidt [rdi]
    ret

LoadCR3:
    mov rax, rdi
    mov cr3, rax
    ret

ReadCR2:
    mov rax, cr2
    ret

ProcessStart:
    mov rsp, rdi        ; Set RSP point to process stack frame.
    jmp TrapReturn      ; After trap return, we we running in process code.