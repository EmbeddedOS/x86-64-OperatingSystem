[BITS 16]
[ORG 0x7e00]

Start:
    ; 1. Get DriveID from boot-code.
    mov [DriveID], dl

    ; 2. Check CPU support getting "Extended Processor Signature and Extended
    ; Feature Bits." information.
    mov eax, 0x80000000     ; Get information "EAX Maximum Input Value for
                            ; Extended Function CPUID Information." of CPU.
    cpuid
    cmp eax, 0x80000001
    jb NotSupport           ; Jump if maximum input value is less than
                            ; 0x80000001, the information that we want to get.

    ; 3. Check if long mode is supported or not.
    mov eax, 0x80000001     ; We will get CPU information: "Extended Processor
                            ; Signature and Extended Feature Bits."
    cpuid
    test edx, (1<<29)       ; Bit 29: Intel® 64 Architecture available if 1.
    jz NotSupport           ; If zero flag is set, CPU doesn't support.
    test edx, (1<<26)       ; Bit 26: 1-GByte pages are available if 1.
    jz NotSupport           ; If zero flag is set, CPU doesn't support.

    ; 4. Load the kernel file to address 0x0010000.
LoadKernel:
    mov si, ReadPacket
    mov word[si], 0x10          ; Packet size is 16 bytes.
    mov word[si + 2], 0x64      ; We will load 100 sectors from the disk.
    mov word[si + 4], 0x00      ; Memory offset.
    mov word[si + 6], 0x1000    ; Memory segment. So, we will load the kernel
                                ; code to physical memory at address: 0x1000 *
                                ; 0x10 + 0x00 = 0x100000
    mov dword[si + 8], 0x06     ; We load from sector 7 from hard disk image to
    mov dword[si + 12], 0x00    ; sector 107.

    mov dl, [DriveID]           ; DriveID param.
    mov ah, 0x42                ; Use INT 13 Extensions - EXTENDED READ service.
    int 0x13                    ; Call the Disk Service.
    jc ReadError                ; Carry flag will be set if error.

GetMemoryInfoStart:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 0x14
    mov edi, 0x9000
    xor ebx, ebx
    int 0x15
    jc NotSupport

GetMemoryInfo:
    add edi, 0x14
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 0x14
    int 0x15
    jc GetMemoryDone
    test ebx, ebx

    jnz GetMemoryInfo

GetMemoryDone:
TestA20lLine:
    mov ax, 0xFFFF
    mov es, ax
    mov word[ds:0x7C00], 0xA200
    cmp word[es:0x7C10], 0xA200
    jne SetA20LineDone
    mov word[0x7C00], 0xB200
    cmp word[es:0x7C10], 0xB200
    je NotSupport 

SetA20LineDone:
    xor ax, ax
    mov es, ax

SetVideoMode:
    mov ax, 0x03
    int 0x10

    mov si, Message
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov cx, MessageLen

PrintMessage:
    mov al, [si]
    mov [es:di], al
    mov byte[es:di+1], 0x0A
    add di, 0x02
    add si, 0x01
    loop PrintMessage

; Halt CPU if we encounter some errors.
ReadError:
NotSupport:
End:
    hlt
    jmp End

DriveID:        db 0
Message:        db "Set text mode."
MessageLen:     equ $-Message
ReadPacket:     times 16 db 0
