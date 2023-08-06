section .text
global Write

Write:
    ; 1. Allocate 24 bytes space on the stack for three arguments.
    sub rsp, 0x18

    ; 2. Prepare arguments.
    mov eax, 0              ; System call number.
    mov [rsp], rdi          ; Copy first argument to the stack (x86-64 calling
                            ; conventions).
    mov [rsp + 0x08], rsi   ; Copy second argument to the stack.
    mov [rsp + 0x10], rdx   ; Copy third argument to the stack.

    mov rdi, 0x03           ; `rdi` hold number of arguments.
    mov rsi, rsp            ; `rsi` hold stack pointer, so now `rsi` points to
                            ; the address of the arguments.
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller. Response from kernel is
    ; saved at `rax` register.
    add rsp, 0x18
    ret