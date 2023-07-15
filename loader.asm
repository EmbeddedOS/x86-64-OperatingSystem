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

    ; 5. Get system memory map, let get first 20 bytes.
GetMemoryInfoStart:
    mov eax, 0xE820         ; Configure param for GET SYSTEM MEMORY MAP service.
    mov edx, 0x534D4150     ; Configure param for GET SYSTEM MEMORY MAP service.
    mov ecx, 0x14           ; Size of buffer for result, in bytes.
    mov edi, 0x9000         ; ES:DI -> buffer result.
    xor ebx, ebx            ; 0x00 to start at beginning of map.
    int 0x15                ; Call the BIOS service.
    jc NotSupport           ; Carry flag will be set if error.

GetMemoryInfo:
    add edi, 0x14           ; Point DI to next 20 bytes to receive next memory
                            ; block.
    mov eax, 0xE820         ; Configure param for GET SYSTEM MEMORY MAP service.
    mov edx, 0x534D4150     ; Configure param for GET SYSTEM MEMORY MAP service.
    mov ecx, 0x14           ; Size of buffer for result, in bytes.
    int 0x15                ; Call the BIOS service.
    jc GetMemoryDone        ; Carry flag will be set if error. But if it is set,
                            ; this means, the end of memory blocks has already
                            ; been reached.
    test ebx, ebx           ; If ebx is none zero, that means we don't reach the
    jnz GetMemoryInfo       ; end of list, so we need to get next block.

GetMemoryDone:
    ; 6. Check if A20 line is enabled on machine or not.
TestA20lLine:
    mov ax, 0xFFFF
    mov es, ax                      ; Set extra segment to 0xFFFF
    mov word[ds:0x7C00], 0xA200     ; Set 0xA200 to address 0x7C00.
    cmp word[es:0x7C10], 0xA200     ; Compare value at address 0x107C00 with
                                    ; 0xA200.
    jne SetA20LineDone              ; If not equal, means previous instruction
                                    ; actually reference to address 0x107C00.
    mov word[0x7C00], 0xB200        ; If equal, will make a test with different
    cmp word[es:0x7C10], 0xB200     ; value to confirm.
    je NotSupport                   ; If it actually access same the memory
                                    ; location, that means the machine actually
                                    ; disable A20 line, so we will exit.

    ; 7. Clear Extra segment register.
SetA20LineDone:
    xor ax, ax
    mov es, ax

    ; 8. Set video mode.
SetVideoMode:
    mov ax, 0x03            ; AH=0x00 use BIOS VIDEO - SET VIDEO MODE service.
                            ; AL=0x03 use the base address to print at 0xB8000.
    int 0x10                ; Call the service.

    mov si, Message         ; Point SI to message.
    mov ax, 0xB800
    mov es, ax              ; Set extra segment to 0xB800, so memory we access
                            ; via this register will be: 0xB8000 + offset
    xor di, di
    mov cx, MessageLen

PrintMessage:
    mov al, [si]
    mov [es:di], al         ; Copy character to screen address.
    mov byte[es:di+1], 0x0A ; Copy attribute byte of character also.
    add di, 0x02            ; di point to next position on screen.
    add si, 0x01            ; si point to next byte in message.
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
