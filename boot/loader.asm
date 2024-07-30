; The loader code is put on sectors [1:5] in our hard disk image, after the BIOS
; found the boot sector, we will use the boot code to load the loader code into
; physical memory at address 0x7E00. First of all, to prepare to long mode, we
; need to check it is supported or not. That is done by using `cpuid`
; instruction and it's service: "EAX Maximum Input Value for Extended Function 
; CPUID Information.". After that we load 100 sectors [6:105] which we have
; spent for our kernel code (51200 bytes are enough for our kernel code). Now
; the physical memory look like:
;              Memory
;      |-------------------| Max size
;      |      Free         |
;      |-------------------|0x00100000
;      |      Reserved     |
;      |-------------------| 0x80000
;      |      Free         |
;      |                   | 0x10000 -> We will use this region for kernel code.
;      |-------------------|
;      |      Loader       | 0x7E00
;      |-------------------|
;      |     MBR code      | 0x7C00
;      |-------------------|
;      |      Free         | -> We used this region for stack.
;      |-------------------|
;      | BIOS data vectors |
;      |-------------------| 0
;
; After that, we will check everything is fine for protected mode such as: valid
; memory, A20 is already enable, etc. So, one thing we need to note is set video
; mode, because after we switch to protected mode, we can not use BIOS service
; anymore, so we need set video mode, and from that point, we can print to
; screen via the video base address 0xB8000. Next thing we need to do is switch
; to Protected Mode. To do this we need to prepare some thing:
; - Load Global Descriptor Table: Main purpose of this table is management
;   segment, decentralize permission to segments, set attributes to them such as
;   readable, wriable, etc.
; - Load Interrupt Descriptor Table: Register handler for interrupt lines.
; But we will not deal with interrupts until we switch to Long Mode. So we will
; load LDT with a NULL table. After switch to Protected Mode, we need to jump it
; code segment with Protected Mode entry. PM provides a way to jump to segments,
; `Segment Selector` that represents for index, permission, etc. of segment in
; GDT. For example, the code segment descriptor is index 1 in the GDT, we have
; selector equal 0b00001000 = 0x08.
; The main purpose of protected mode in our system is preparing for Long mode,
; because we can not jump from 16 bit mode to 64 bit mode, that is impossible.
; So after we, switch to PM, we immediately prepare for long mode, there is list
; job we need to do:
; - Prepare for paging: why we need to do that? because we need to enable paging
;   , and this require a structure to manage the current page. We will not run
;   run on PM mode so we will load it with a dummy structure, we do that by
;   selecting a free region about 0x70000 up to 0x80000.
; - Load GDT: in long mode, almost fields at segment descriptor will be ignored.
;   we will only set first entry is code segment (in Long mode, segmentation is 
;   disabled), `ds`, `es`, `ss` are ignored also.
; - Enable long mode by setting bits in special registers (cr4, cr3, msr).
; - Enable paging by setting bit[31] in cr0 register.
; After switch to long mode, final missions of loader code are relocated kernel
; code from address 0x10000 to 0x200000 and finally jump to it. So from now we
; can run kernel code and user application in 64 bit mode. Congratulation!

[BITS 16]
[ORG 0x7E00]

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
                                ; 0x10 + 0x00 = 0x10000
    mov dword[si + 8], 0x06     ; We load from sector 7 from hard disk image to
    mov dword[si + 12], 0x00    ; sector 107.

    mov dl, [DriveID]           ; DriveID param.
    mov ah, 0x42                ; Use INT 13 Extensions - EXTENDED READ service.
    int 0x13                    ; Call the Disk Service.
    jc ReadError                ; Carry flag will be set if error.

; Load the shell process to 0x20000 to run init process.
LoadShell:
    mov si, ReadPacket
    mov word[si], 0x10          ; Packet size is 16 bytes.
    mov word[si + 2], 0x14      ; We will load 20 sectors from the disk.
    mov word[si + 4], 0x00      ; Memory offset.
    mov word[si + 6], 0x2000    ; Memory segment. So, we will load the user
                                ; code to physical memory at address: 0x2000 *
                                ; 0x10 + 0x00 = 0x20000
    mov dword[si + 8], 0x6A     ; We load from sector 107 from hard disk image
    mov dword[si + 12], 0x00    ; to sector 127.

    mov dl, [DriveID]           ; DriveID param.
    mov ah, 0x42                ; Use INT 13 Extensions - EXTENDED READ service.
    int 0x13                    ; Call the Disk Service.
    jc ReadError                ; Carry flag will be set if error.

    ; 5. Get system memory map, let get first 20 bytes.
GetMemoryInfoStart:
    mov eax, 0xE820         ; Configure param for GET SYSTEM MEMORY MAP service.
    mov edx, 0x534D4150     ; Configure param for GET SYSTEM MEMORY MAP service.
    mov ecx, 0x14           ; Size of buffer for result, in bytes.
    mov dword[0x9000], 0    ; We will save count of structures at address 0x9000
                            ; , and store structures start at 0x9008.
    mov edi, 0x9008         ; ES:DI -> buffer result.
    xor ebx, ebx            ; 0x00 to start at beginning of map.
    int 0x15                ; Call the BIOS service.
    jc NotSupport           ; Carry flag will be set if error.

GetMemoryInfo:
    inc dword[0x9000]       ; Increase count of memory blocks, that we actually
                            ; get the information.
    test ebx, ebx           ; If ebx is zero, that means we don't reach the
    jz GetMemoryDone        ; end of list, so we need to get next block.

    add edi, 0x14           ; Point DI to next 20 bytes to receive next memory
                            ; block.
    mov eax, 0xE820         ; Configure param for GET SYSTEM MEMORY MAP service.
    mov edx, 0x534D4150     ; Configure param for GET SYSTEM MEMORY MAP service.
    mov ecx, 0x14           ; Size of buffer for result, in bytes.
    int 0x15                ; Call the BIOS service.
    jnc GetMemoryInfo       ; Carry flag will be set if error. But if it is set,
                            ; this means, the end of memory blocks has already
                            ; been reached.

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

    ; 9. Switch to protected mode to prepare for long mode.
SwitchToProtectedMode:
    cli                     ; Disable interrupts.
    lgdt [GDT32Pointer]     ; Load global descriptor table.
    lidt [IDT32Pointer]     ; Load an invalid IDT (NULL) because we don't deal
                            ; with interrupt until switching to Long Mode.

    mov eax, cr0            ; We enable Protected Mode by set bit 0 of Control
    or eax, 0x01            ; register, that will change the processor behavior.
    mov cr0, eax

    jmp 0x08:PMEntry        ; Jump to Protected Mode Entry with selector select 
                            ; index 1 in GDT (code segment descriptor) so 
                            ; segment selector: Index=000000001, TI=0, RPL=00

; Halt CPU if we encounter some errors.
ReadError:
NotSupport:
End:
    hlt
    jmp End

DriveID:        db 0
ReadPacket:     times 16 db 0

; Global Descriptor Table Structure, we define 3 entries with 8 bytes for each.
GDT32:
    dq 0            ; First entry is NULL.
CodeSegDes32:       ; Next entry is Code Segment Descriptor.
    dw 0xFFFF       ; First two byte is segment size, we set to maximum size for
                    ; code segment.
    db 0, 0, 0      ; Next three byte are the lower 24 bits of base address, we
                    ; set to 0, means the code segment starts from 0.
    db 0b10011010   ; Next byte specifies the segment attributes, we will set
                    ; code segment attributes: P=1, DPL=00, S=1, TYPE=1010.
    db 0b11001111   ; Next byte is segment size and attributes, we will set code
                    ; segment attributes and size: G=1,D=1,L=0,A=0,LIMIT=1111.
    db 0            ; Last byte is higher 24 bits of bit address, we set to 0,
                    ; means the code segment starts from 0.
DataSegDes32:       ; Next entry is Data Segment Descriptor. We will set data
                    ; and code segment base on same memory (address + size).
    dw 0xFFFF
    db 0, 0, 0
    db 0b10010010   ; Different between data segment and code segment descriptor
                    ; is the type segment attributes: TYPE=0010 means this a
                    ; WRITABLE segment.
    db 0b11001111
    db 0

GDT32Len: equ $-GDT32

GDT32Pointer: dw GDT32Len - 1   ; First two bytes is GDT length.
              dd GDT32          ; Second is GDT32 address.

IDT32Pointer: dw 0              ; First two bytes is IDT length.
              dd 0              ; Second is IDT32 address.

[BITS 32]
PMEntry:
    ; 10. Initialize segment registers to data segment descriptor entry, the
    ; data segment descriptor is third entry in GDT so we have segment selector:
    ; Index=000000002, TI=0, RPL=00 in binary.
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x7C00

    ; 11. Setup page structure memory for Paging.
    ; This bock of code basically finds a free memory area and initialize the
    ; paging structure which is used to translate the virtual address to 
    ; physical. The addresses (0x80000 to 0x90000) may used for BIOS data. We 
    ; can use memory area from 0x70000 to 0x80000.
    cld
    mov edi, 0x70000        ; First off, we zero the 10000 bytes of memory
    xor eax, eax            ; region starting from 0x70000.
    mov ecx, 0x10000/4
    rep stosd

    mov dword[0x70000], 0x71003     ; Each entry in the Page Map Level 4 table
                                    ; represent 512GB and we only implement the
                                    ; low 1GB. So we setup the first entry of
                                    ; of the table. Each table takes up 4KB
                                    ; space, since the table include 512 entries
                                    ; with each entry being 8 bytes. The next
                                    ; table address is set to 0x1000 and the
                                    ; lower 3 bits are the attribute we need to
                                    ; set.
    mov dword[0x71000], 0b10000011  ; In the Page directory pointer table, we
                                    ; also setup the first entry. Because each
                                    ; entry here points to 1GB physical page,
                                    ; and this is all we need. The base address
                                    ; of the physical page is set to 0 and the
                                    ; attribute is set to 0b10000011.
    ; Remap kernel from 0x200000 in physical to 0xFFFF800000000000 in virtual.
    mov eax, (0xFFFF800000000000>>39)   ; We retrive the 9 bit Page Map Level 4
                                        ; located at the bit 39 of the address.
    and eax, 0x1FF                      ; And the we clear other bits. So we get
                                        ; 9 bit index value.
                                        ; Next we use the index to locate the
                                        ; corresponding entry in the table. Each
                                        ;entry takes up 8 bytes.
    mov dword[0x70000 + eax*8], 0x72003 ; The value we assign is 0x72003 which
                                        ; means the next table is at 0x72000 and
                                        ; the attributes are same as before.
    mov dword[0x72000], 0b10000011
    ; 12. Load GDT for 64-bit mode.
    lgdt [GDT64Pointer]

    ; 13. Enable 64-bit mode and jump to long mode entry.
    mov eax, cr4            ; Set bit 5 Physical Address Extension of Control
    or eax, (1<<5)          ; register 4.
    mov cr4, eax

    mov eax, 0x70000        ; Setup the page structure we just setup 0x70000 to
    mov cr3, eax            ; cr3 Control Register 3. So the Address of Page Map
                            ; Level 4 table is 0x70000. Each entry in the Page
                            ; Map Level 4 table represent 512GB and we only
                            ; implement the low 1GB.

    mov ecx, 0xC0000080     ; Enable Long mode, using Extended Feature Enable
    rdmsr                   ; Register, it is a special register. We need to get
    or eax, (1 << 8)        ; it via index (0xC0000080) pass to ecx register,
    wrmsr                   ; and then call read MSR instruction, result will
                            ; return to eax. So will set bit[8] in eax to enable
                            ; long mode and write back to the MSR register.
    
    mov eax, cr0            ; Enable Paging by set bit[31] on cr0 Control
    or eax, (1 << 31)       ; Register 0.
    mov cr0, eax

    jmp 0x08:LMEntry        ; Jump to long mode entry using segment selector 8.

; Global Descriptor Table Structure for 64 bit mode, we define 2 entries with 8
; bytes for each.
GDT64:
    dq 0            ; First entry is NULL.
CodeSegDes64:       ; Next entry is Code Segment Descriptor.
                    ; Almost field of the entry are ignored, so we will set them
                    ; to 0.
    dq 0x0020980000000000

GDT64Len: equ $-GDT64

GDT64Pointer: dw GDT64Len - 1   ; First two bytes is GDT length.
              dd GDT64          ; Next four bytes are GDT64 address.

[BITS 64]
LMEntry:
    ; 14. Initialize stack pointer.
    mov rsp, 0x7C00

    ; 15. Relocate kernel from 0x10000 to 0x200000 and jump to it.
    cld                 ; Clear direction flag.
    mov rdi, 0x200000   ; Destination address.
    mov rsi, 0x10000    ; Source address.
    mov rcx, 51200/8    ; RCX acts as a counter, we will copy 100 sectors: 512
                        ; * 100 = 51200 bytes.
    rep movsq           ; Repeat quad-word one time.

    ; Since the kernel is relocated to the new virtual address which is far away
    ; from the loader, we need to save it to the 64-bit register and jump to
    ; kernel.
    mov rax, 0xFFFF800000200000     ; Move virtual address to `rax`.
    jmp rax                         ; Jump to kernel entry.

