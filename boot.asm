; This file is boot sector, it is the first sector of our disk image that will 
; be found be BIOS using partition entries and signature as magic numbers. After
; BIOS load this sector to physical memory at address 0x7C00, the BIOS transfer
; control to our boot code, but this sector is very small (just 512 bytes). So
; we need to use this sector to load more sectors to physical memory, so from
; that, we can do many things such as load kernel code, run user tasks, etc.
; What this code do simply are initialize segment registers, set stack pointer
; point to 0x7C00 (the stack pointer will grows downwards, so, we don't need to
; care about memory corruption to our boot code), and finally this code check if
; Disk Extension Service is supported or not, if it's, we use the Service to
; read next sector (loader code) from our disk to physical memory at address 
; 0x7E00 and transfer control to it. After all, the physical memory look like:
;         Physical Memory
;      |-------------------| Max size
;      |                   |
;      |-------------------|
;      |      Loader       |
;      |-------------------| 0x7E00
;      |     MBR code      |
;      |-------------------| 0x7C00 <--SP
;      |       STACK       |           ||
;      |                   |           ||
;      |                   |           \/
;      |-------------------| 0
; Here we go!

[BITS 16]
[ORG 0x7C00]    ; The boot code start at address 0x7C00 and ascending.

Start:
    ; 1. Clear segment registers.
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; 2. Stack pointer start at address 0x7c00 and grows downwards.
    mov sp, 0x7C00

    ; 3. Test Disk Extension Service.
TestDiskExtension:
    mov [DriveID], dl
    mov ah, 0x41        ; Use INT 13 Extensions - INSTALLATION CHECK service.
    mov bx, 0x55AA
    int 0x13            ; Call the Disk Service.
    jc NotSupport       ; CF - Carry Flag is set on error or clear on success.
    cmp bx, 0xAA55      ; bx = 0xAA55 if installed.
    jne NotSupport

    ; 4. Load the loader.
LoadLoader:
    mov si, ReadPacket          ; Load the packet address to si.
    mov word[si], 0x10          ; Packet size is 16 bytes.
    mov word[si + 2], 0x05      ; We we load 5 sectors which is enough space for
                                ; our loader.
    mov word[si + 4], 0x7E00    ; Offset which we want to read loader file.
    mov word[si + 6], 0x00      ; Segment, the logical memory to load the file
                                ; is: 0x00 * 0x10 + 0x7E00 = 0x7E00
    mov dword[si + 8], 0x01     ; 32 bit low address of LBA.
    mov dword[si + 12], 0x00    ; 32 bit high address of LBA.
                                ; We will start at sector 2 but set 1 to LBA
                                ; Because the LBA is zero-based address.

    mov dl, [DriveID]           ; DriveID param.
    mov ah, 0x42                ; Use INT 13 Extensions - EXTENDED READ service.
    int 0x13                    ; Call the Disk Service.
    jc ReadError                ; Carry flag will be set if error.

    ; 5. Loader code has been loaded to physical memory, jump to loader code and 
    ; transfer control to it.
    mov dl, [DriveID]           ; Pass the DriveID to the loader.
    jmp 0x7E00

NotSupport:
ReadError:
    mov ah, 0x13
    mov al, 1
    mov bx, 0xA
    xor dx, dx
    mov bp, Message
    mov cx, MessageLen
    int 0x10

; Halt CPU if we encounter some errors.
End:
    hlt
    jmp End

DriveID:            db 0
Message:            db "An error is occured when booting system."
MessageLen:         equ $-Message

; Disk Address Packet Structure.
ReadPacket:         times 16 db 0

; Clear memory from current address to address at byte 446th (0x1BE).
times (0x1BE-($-$$)) db 0

; End of boot sector, we need 16 * 4 = 64 bytes for 4 partition entries. Some
; BIOS will try to find the valid partition entries. We want the BIOS treat our
; image as a hard disk and boot from them, so we need to define these entries.
; The first  partition entry:
db 0x80                     ; Boot indicator, 0x80 means boot-able partion.
db 0, 2, 0                  ; Starting of CHS value (Cylinder, Head, Sector).
db 0xF0                     ; Type of sector.
db 0xFF, 0xFF, 0xFF         ; Ending of CHS value (Cylinder, Head, Sector).
dd 1                        ; Starting sector.
dd (20*16*63 - 1)           ; Size of our disk: 10MB.

; Other entries are set to 0.
times (16*3) db 0

; Final two bytes are signature of the boot sector - magic numbers.
db 0x55
db 0xaa