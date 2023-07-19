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
    ; 1. Setup handlers for IDT.
    mov rdi, IDT64
    mov rax, DivideBy0Handler   ; Setup Divide By 0 handler.
    call SetIntHandler

    add rdi, 32 * 16            ; Point to IRQ 0.
    mov rax, Timer0Handler      ; Setup channel 0 timer handler.
    call SetIntHandler

    add rdi, 32 * 16 + 7 * 16   ; Point to IRQ 7.
    mov rax, SIRQ               ; Setup spurious interrupt handler.
    call SetIntHandler

    ; 2. Load GDT and IDT.
    lgdt [GDT64Pointer]
    lidt [IDT64Pointer]

    ; 3. Set Task state segment.
SetTSS:
    mov rax, TSS                    ; Point to TSS structure.
    mov [TaskStateSegDes64 + 2], ax
    shr rax, 16
    mov [TaskStateSegDes64 + 4], al
    shr rax, 8
    mov [TaskStateSegDes64 + 7], al
    shr rax, 8
    mov [TaskStateSegDes64 + 8], eax
    mov ax, 0x20                    ; Tss des is 5th entry, so we move 0x20.
    ltr ax                          ; Load TSS.

    ; 4. Load code segment descriptor to cs register.
    push 0x08           ; Push Code Selector.
    push KernelEntry    ; Push Kernel entry address.
    db 0x48             ; Set operand-size to 64 bit.
    retf                ; we load code segment descriptor by far return to
                        ; `caller` with caller address is KernelEntry.

KernelEntry:
    ; 5. Initialize PIT - Programable Interval Timer.
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

    mov al, 0b11111110  ; Masking all interrupts, except IRQ0 for the timer
    out 0x21, al        ; interrupt we used.
    mov al, 0b11111111
    out 0xA1, al

    ; 5. Enable interrupts.
    ;sti

    ;6. Test switch to ring 3 using iretq, we push fake stack frame.
    push 0x18|3     ; ss selector.
    push 0x7C00     ; RSP.
    push 0x202      ; Rflags, we push 0x202 to enable interrupt via state reg
                    ; when we get from ring 0 to ring 3.
    push 0x10|3     ; cs selector.
    push UserEntry  ; RIP.

    iretq

KernelEnd:
    hlt
    jmp KernelEnd

SetIntHandler:
    mov [rdi], ax
    shr rax, 16
    mov [rdi + 6], ax
    shr rax, 16
    mov [rdi + 8], eax
    ret

UserEntry:
    ; 7. Check current ring.
    mov ax, cs
    and al, 0b11
    cmp al, 0b11
    jne UserEnd

    mov byte[0xB8012], '3'
    mov byte[0xB8013], 0xA

UserEnd:
    jmp UserEnd

DivideBy0Handler:
    ; Save state of CPU, we will save 15 general-purpose registers.
    push rax
    push rbx
    push rcx
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

    mov byte[0xB8002], 'D'
    mov byte[0xB8003], 0xA
    jmp KernelEnd

    ; Restore state of CPU.
    pop r15
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

    iretq

Timer0Handler:
    ; Save state of CPU, we will save 15 general-purpose registers.
    push rax
    push rbx
    push rcx
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

    inc byte[0xB8004]
    mov byte[0xB8005], 0xA
    mov al, 0b00100000  ; Set bit 5.
    out 0x20, al        ; ACK the interrupt to run timer again, so the timer
                        ; interrupt will fire periodically.

    ; Restore state of CPU.
    pop r15
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

    iretq

SIRQ:
    ; Save state of CPU, we will save 15 general-purpose registers.
    push rax
    push rbx
    push rcx
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

    ; Check the interrupt is spurious interrupt or not.

    mov al, 0b00001011  ; Send read ISR register command.
    out 0x20, al
    in al, 0x20
    test al, (1<<7)
    jz ExitSpuriousInt  ; If it is spurious int, we exit and not re-enable this
                        ; interrupt by don't set the end interrupt ack.

    mov al, 0x20
    out 0x20, al    ; Send ack make end of interrupt if it is regular interrupt.

    ExitSpuriousInt:
    ; Restore state of CPU.
    pop r15
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

    iretq

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

; Task state segment structure - 128 bytes.
TSS:
    dd 0            ; First four bytes is reserved.
    dq 0x150000     ; Set RSP0 to new address.
    times 88 db 0   ; Clear next 88 bytes to 0.
    dd TssLen       ; IO permission bitmap, assign size of TSS means we don't
                    ; use the IO permission bitmap.
TssLen: equ $-TSS