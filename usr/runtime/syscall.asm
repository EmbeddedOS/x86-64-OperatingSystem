section .text
global syscall0
global syscall1
global syscall2
global syscall3
global syscall4
global syscall5

syscall0:
    mov rax, rdi
    mov rdi, 0
    int 0x80
    ret

syscall1:
    ; 1. Allocate 8 bytes space on the stack for one argument.
    sub rsp, 0x08

    ; 2. Prepare arguments.
    mov rax, rdi            ; System call number.
    mov [rsp], rsi          ; Copy first argument to the stack.
    mov rdi, 0x01
    mov rsi, rsp
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller.
    add rsp, 0x08
    ret

syscall2:
    ; 1. Allocate 16 bytes space on the stack for two arguments.
    sub rsp, 0x10

    ; 2. Prepare arguments.
    mov rax, rdi            ; System call number.
    mov [rsp], rsi          ; Copy first argument to the stack.
    mov [rsp + 0x08], rdx   ; Copy second argument to the stack.
    mov rdi, 0x02
    mov rsi, rsp
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller.
    add rsp, 0x10
    ret

syscall3:
    ; 1. Allocate 24 bytes space on the stack for three arguments.
    sub rsp, 0x18

    ; 2. Prepare arguments.
    mov rax, rdi            ; System call number.
    mov [rsp], rsi          ; Copy first argument to the stack.
    mov [rsp + 0x08], rdx   ; Copy second argument to the stack.
    mov [rsp + 0x10], rcx   ; Copy third argument to the stack.

    mov rdi, 0x03
    mov rsi, rsp
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller.
    add rsp, 0x18
    ret

syscall4:
    ; 1. Allocate 32 bytes space on the stack for three arguments.
    sub rsp, 0x20

    ; 2. Prepare arguments.
    mov rax, rdi            ; System call number.
    mov [rsp], rsi          ; Copy first argument to the stack.
    mov [rsp + 0x08], rdx   ; Copy second argument to the stack.
    mov [rsp + 0x10], rcx   ; Copy third argument to the stack.
    mov [rsp + 0x18], r8    ; Copy 4th argument to the stack.

    mov rdi, 0x04
    mov rsi, rsp
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller.
    add rsp, 0x20
    ret

syscall5:
    ; 1. Allocate 40 bytes space on the stack for three arguments.
    sub rsp, 0x28

    ; 2. Prepare arguments.
    mov rax, rdi            ; System call number.
    mov [rsp], rsi          ; Copy first argument to the stack.
    mov [rsp + 0x08], rdx   ; Copy second argument to the stack.
    mov [rsp + 0x10], rcx   ; Copy third argument to the stack.
    mov [rsp + 0x18], r8    ; Copy 4th argument to the stack.
    mov [rsp + 0x20], r9    ; Copy 5th argument to the stack.

    mov rdi, 0x05
    mov rsi, rsp
    
    ; 3. Execute syscall interrupt instruction.
    int 0x80

    ; 4. Restore the stack and return to the caller.
    add rsp, 0x28
    ret