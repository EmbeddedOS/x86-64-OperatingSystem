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
;
; About the file system, We use FAT16 File System, The 16-bit part describes the
; way units are allocated on the drive. The FAT16 file system uses a 16-bit
; number to identify each allocation unit (called cluster), and this gives it a
; total of 65.536 clusters. The size of each cluster is defined in the boot
; sector of the volume (volume = partition). The File System ID number usually
; associated with FAT16 volumes are 04h and 06h. The first is used on volumes
; with less than 65536 sectors (typical this is on drives less than 32 Mb in
; size), and the latter one is used on volumes with more than 65536 sectors.
;
; The FAT16 file system structure contains the following regions:
; + Reserved Region (incl. Boot Sector).
; + File Allocation Table (FAT).
; + Root Directory.
; + Data Region.
;
; The first sector (boot sector) contain information which is used to calculate
; the sizes and locations of the other regions. The boot sector also contain
; code to boot the operating system installed on the volume. The DATA REGION is
; split up into logical blocks called clusters. Each of these clusters has an
; accompanying entry in the FAT REGION. The cluster specific entry can either
; contain a value of the next cluster which contain data from the file, or a so
; called End-of-file value which means that there are no more clusters which
; contain data from the file.
; The ROOT DIRECTORY and its sub-directories contain filename, dates, attribute
; flags and starting cluster information about the filesystem objects.
; 
; The RESERVED REGION will start from the sector that contains BIOS Prameter
; Block until value of `Reserved sectors`. In our image, it start from first
; sector.
;
; The first sector in the RESERVED REGION is the boot sector. Though this sector
; is typical 512 bytes in can be longer depending on the media. The boot sector
; typical start with a 3 byte jump instruction to where the bootstrap code is
; stored, followed by an 8 byte long string set by the creating operating
; system. This is followed by the BIOS Parameter Block, and then by an Extended
; BIOS Parameter Block. Finally the boot sector contain boot code and a
; signature.
; 
; In the RESERVED REGION we will store all kernel code (boot, loader, kernel,
; etc).
; 
; The Boot Sector structure:
; |   Part    |Offset|Size|                     Description                    |
; |-----------|------|----|----------------------------------------------------|
; |   Code    |0x0000| 3  | Code to jump to the bootstrap code.                |
; |-----------|------|----|----------------------------------------------------|
; | OS name   |0x0003| 8  | Oem Id - Name of the formatting OS.                |
; |-----------|------|----|----------------------------------------------------|
; |    BIOS   |0x000B| 2  | Bytes per sector. This value is the number of bytes|
; | parameter |      |    | in each physical sector. The allowed values are:   |
; |   Block   |      |    | 512, 1024, 2048 or 4096 bytes.                     |
; |           |0x000D| 1  | Sectors per Cluster. The allowed values are: 1, 2, |
; |           |      |    | 4, 8, 16, 32 or 128.                               |
; |           |0x000E| 2  | Reserved sectors from the start of the volume.     |
; |           |      |    | Since the reserved region always contain the boot  |
; |           |      |    | sector a zero value in this field is not allowed.  |
; |           |      |    | The usual setting of this value is 1. The value is |
; |           |      |    | used to calculate the location for the first sector|
; |           |      |    | containing the FAT.                                |
; |           |0x0010| 1  | Number of FAT copies - Usual 2 copies are used to  |
; |           |      |    | prevent data loss. The usage of two copies are to  |
; |           |      |    | prevent data loss if one or part of one FAT copy is|
; |           |      |    | corrupted.                                         |
; |           |0x0011| 2  | Number of possible root entries - 512 entries are  |
; |           |      |    | recommended.                                       |
; |           |0x0013| 2  | Small number of sectors - Used when volume size is |
; |           |      |    | less than 32 Mb. This field states the total number|
; |           |      |    | of sectors in the volume.                          |
; |           |0x0015| 1  | Media Descriptor:                                  |
; |           |      |    |  Hex    Capacity        Physical Format            |
; |           |      |    | + F0    2.88 MB     3.5-inch, 2-sided, 36-sector   |
; |           |      |    | + F0    1.44 MB     3.5-inch, 2-sided, 18-sector   |
; |           |      |    | + F8       ?                Fixed disk             |
; |           |      |    | + F9    720 KB      3.5-inch, 2-sided, 9-sector    |
; |           |      |    | + F9    1.2 MB     5.25-inch, 2-sided, 15-sector   |
; |           |      |    | + FA       ?                    ?                  |
; |           |      |    | + FB       ?                    ?                  |
; |           |      |    | + FC    180 KB     5.25-inch, 1-sided, 9-sector    |
; |           |      |    | + FD    360 KB     5.25-inch, 2-sided, 9-sector    |
; |           |      |    | + FE    160 KB     5.25-inch, 1-sided, 8-sector    |
; |           |      |    | + FF    320 KB     5.25-inch, 2-sided, 9-sector    |
; |           |0x0016| 2  | Sector per FAT. This is the number of sectors      |
; |           |      |    | occupied by one copy of the FAT.                   |
; |           |0x0018| 2  | Sectors per Track. This value is used when the     |
; |           |      |    | volume is on a media which have a geometry, that is|
; |           |      |    | when the LBA number is broken down into a          |
; |           |      |    | Cylinder-Head-Sector address. This field represents|
; |           |      |    | the multiple of the max.                           |
; |           |0x001A| 2  | Number of Heads. This value is used when the volume|
; |           |      |    | is on a media which have a geometry, that is when  |
; |           |      |    | the LBA number is broken down into a               |
; |           |      |    | Cylinder-Head-Sector address. This field represents|
; |           |      |    | the Head value used when the volume was formatted. |
; |           |0x001C| 4  | Hidden Sectors. When the volume is on a media that |
; |           |      |    | is partitioned, this value contains the number of  |
; |           |      |    | sectors preceeding the first sector of the volume. |
; |           |0x0020| 4  | Large number of sectors - Used when volume size is |
; |           |      |    | greater than 32 Mb.                                |
; |-----------|------|----|----------------------------------------------------|
; | Extended  |0x0024| 1  | Drive Number. Used by some bootstrap code, fx.     |
; |    BIOS   |      |    | MS-DOS. This is the int 13h drive number of the    |
; | parameter |      |    | drive. The value 0x00 is used for the first floppy |
; |   Block   |      |    | drive and the value 0x80 is used for the first     |
; |           |      |    | harddrive.                                         |
; |           |0x0025| 1  | Reserved. Is used by Windows NT to decide if it    |
; |           |      |    | shall check disk integrity.                        |
; |           |0x0026| 1  | Extended Boot Signature. Indicates that the next   |
; |           |      |    | three fields are available.                        |
; |           |0x0027| 4  | Volume Serial Number. This value is a 32 bit random|
; |           |      |    | number, which, combined with the volume label,     |
; |           |      |    | makes is possible to track removable media and     |
; |           |      |    | check if the correct one is inserted.              |
; |           |0x002B| 11 | Volume Label. Should be the same as in the root    |
; |           |      |    | directory.                                         |
; |           |0x0036| 8  | File System Type. The string should be 'FAT16 '.   |
; |-----------|------|----|----------------------------------------------------|
; |   Code    |0x003E|448 | Bootstrap code. May schrink in the future.         |
; |-----------|------|----|----------------------------------------------------|
; | Signature |0x00FE| 2  | Boot sector signature. This is the AA55h signature.|
; |-----------|------|----|----------------------------------------------------|
;
; This is our image structure 100MB (0x00000000->0x06400000):
;    Address|        FAT 16   |SECTOR|           Description                   |
; 0x00000000| BPB REGION      |  1   | Boot sector.                            |
; 0x00000200| RESERVED REGION | 200  | Our kernel code.                        |
; 0x00019000| FAT REGION      | 400  | FAT.                                    |
; 0x0004B000| ROOT DIRECTORY  |      | Contain directory entries.              |
; ..........|                 |      |                                         |
; ..........| DATA REGION     |      |                                         |
; ..........|                 |      |                                         |
; 0x06400000|                 |      |                                         |
;
; The ROOT DIRECTORY REGION size depends on number of entries, each entry take
; 32 bytes. And with start cluster number in each entry structure, and if data
; of entry is large than cluster size we can use FAT table to get all clusters
; which data lie on.

[BITS 16]
[ORG 0x7C00]    ; The boot code start at address 0x7C00 and ascending.

; Region 1: Code to jump to the bootstrap code.
jmp short Start
nop

; Region 2: BIOS Parameter Block.
OEMIdetifier db     'LARVAOS '
BytesPerSector      dw 0x200
SectorsPerCluster   db 0x4      ; Each cluster is 2KB.
ReservedSectors     dw 0xC8     ; We reverse first 200 sectors for our kernel.
                                ; So, the FAT REGION will start at sector 201.
FATcopies           db 0x02
RootDirEntries      dw 0x200
NumSectors          dw 0x00
MediaType           db 0xF8
SectorsPerFAT       dw 0xC8
SectorsPerTrack     dw 0x3F
NumberOfHeads       dw 0x10
HiddenSectors       dd 0x00
SectorsBig          dd 0x31F11  ; 100MB for our image.

; Region 3: Extended BIOS Parameter Block.
DriveNumber             db 0x80
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'LARVAOS    '
SystemIDString          db 'FAT16   '

; Region 4: Bootstrap code.
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
    mov word[si + 2], 0x15      ; We we load 15 sectors which is enough space
                                ; for our loader.
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
;times (0x1BE-($-$$)) db 0
times (0x1FE-($-$$)) db 0

; End of boot sector, we need 16 * 4 = 64 bytes for 4 partition entries. Some
; BIOS will try to find the valid partition entries. We want the BIOS treat our
; image as a hard disk and boot from them, so we need to define these entries.
; The first partition entry:
;db 0x80                     ; Boot indicator, 0x80 means boot-able partion.
;db 1, 1, 0                  ; Starting of CHS value (Cylinder, Head, Sector).
;db 0x06                     ; Type of sector.
;db 0x0F, 0x3F, 0xCA         ; Ending of CHS value (Cylinder, Head, Sector).
;dd 0x3F                     ; Starting sector.
;dd 0x31F11                  ; Size of our disk: 10MB.

; Other entries are set to 0.
;times (16*3) db 0

; Region 5: Boot sector signature.
db 0x55
db 0xAA