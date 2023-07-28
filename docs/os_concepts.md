# x86-64 operating system

## I. Introduction

### 3. Intel Core Operating System concepts

- 1. Address space:
  - Address space is set of addresses that a process can use to address memory.
    - 64-bit Address: Each number references a specific address giving us an address space of 2^64 bytes which is really large.
    - Byte Addressing: The smallest addressable unit is byte. So each address references a storage location which is a byte long.

      |   Byte    |2^64 - 1
      |           |...
      |   ...     |...
      |   Byte    |2
      |   Byte    |1
      |   Byte    |0

  - Within this address space, we can keep part of it for RAM and part for hard disk and some other devices.
    - NOTE: the address space is normally larger than the physical memory.

- 2. Process:
  - Process is basically a program in execution. It contains:
    - program instructions.
    - the data, stack, heap within the address space.
    - registers.
    - list of open files.
    - etc.
    -> Process as a container holding all the information in order to run the program.
  - Each process has its address space which process can read and write.
  - Process memory map look like:
    |Kernel (code, data)|
    |-------------------| Max size
    |       Stack       |
    |-------------------|
    |                   |
    |-------------------|
    |        Heap       |
    |-------------------|
    |        Data       |
    |-------------------|
    |   Instruction     |
    |-------------------| 0

  - The address is from 0 to Max size which can be defined by the OS.
    - And the OS kernel resides in memory above the Max size.

  - When we have multiple processes running on the computer, what happens is that each process has its own address space?
    - The memory reference within one process does not affect the address space of other processes.
    - However, **the kernel which resides in the higher part of address space are the same across all the processes**.
  - The Kernel includes the essential information about each process, such as process state, etc.
    - The user processes aren't allowed to access data in kernel space.

  - There are two modes of operations:
    - Kernel mode: OS runs on.
      - In this mode, we can execute all the instructions and access all the hardware.
    - User mode: where they can only execute a subset of the instructions, they cannot access the data in the kernel space.

  - When a program wants to access data in kernel or hardware, it calls a special set of function called **system calls**.
    - The system call acts as an interface between user programs and the OS.
    - When a program calls a system call, the control is transferred to the operating system, OS wil perform the operation the program needs and return to the user program.

### 4. OS structures

- There are two common structures:
  - Monolithic system.
  - Micro-kernel.

- An OS typically contains scheduling, memory management and system calls interface, device drivers, etc.

- Monolithic Structure:
  - Entire OS runs as a large executable program in kernel mode.
  - All modules as a single one.
  - Calling function is very simple, find the correct function which we need and call that function.
  - So it is direct communication between modules and calling any function is efficient.
  - But free to call any function without restrictions makes the system hard to maintain and any module having errors will affect to entire OS.
    - For example, if we encounter an error in memory management module, we will halt the whole system.

- Micro kernel:
  - OS is divided into small modules which runs in user mode.
  - The module (memory management, device drivers, user processes, etc.) run as separate processes in the user mode.
  - the micro kernel which schedules processes, manage communication between these processes are running in the kernel mode. So the **kernel is really small**.
  - Any error in those modules will only affect themselves and will not crash the whole system.

## III. Boot Up

### 9. First program

- BIOS:
  - stands for Basic Input Output System, is usually stored on a small chip on the motherboard.
  - It initializes hardware and provides runtime services for operating systems.
  - When we boot our computer, BIOS initializes and tests the hardware, then find the boot device, read the first sector which is the boot sector from it into memory location `0x7c00` and control is transferred to it.

  - Boot:
    - 1. Power on Self Test (POST), initializes devices.
    - 2. Find boot device and read the first sector from it into memory location `0x7c00`.
    - 3. Jump to 0x7c00.

              Memory
      |-------------------| Max size
      |                   |
      |                   |
      |-------------------|
      |     MBR code      | 0x7c00
      |-------------------|
      |                   |
      |                   |
      |-------------------| 0

  - The information on the first sector is also known as Master Boot Record.

- BIOS also provides low-level services or functions for us to use and these functions become unavailable in protected mode and long mode.
  - Print characters.
  - Disk services.
  - Memory map.
  - Video mode.

- When booting our OS, we will go through three different modes:
  - 1. Read mode: boot process.
  - 2. Protected mode: prepare for long mode.
  - 3. Long mode: consists of two sub modes:
    - 64-bit mode.
    - Compatibility mode.
      - In this mode we can run 16-bit or 32-bit programs under 64-bit OS without recompiling those programs.

- UEFI:
  - Unified Extensible Firmware interface.
  - The Modern computers also come with UEFI.

- Real mode:
  - Load kernel in memory.
  - Get information about hardware.
  - In read mode, there are four special registers called segment registers which are 16 bits registers:
    - `cs` - Code segment
    - `ds` - Data Segment
    - `es` - Extra Segment
    - `ss` - Stack Segment
  - Address Format: The format of an address is the segment register, then colon and offset
    - Segment Register: Offset (logical address)

  - How do we get the physical address?
    - We shift the value in segment register left by 4 bits (that actually is multiply the value by 16 and then add the offset to the result which produces 20-bit physical address)
    - Segment Register * 16 + offset = physical address.

  - For examples:
    - this instruction will read the value in the address at `ds` to `ax`:
      - `mov ax, [ds:50h]` and suppose we have 0x1000 in `ds` register.
      - The memory we get is 0x1000*16 + 0x50 = 0x10050.
      - So this instruction will copy data in address at 0x10050 to `ax` register.
      - If you dont specify `ds` in this command, it is still used as **DEFAULT REGISTER.**

    - Suppose, `ss` holds the value 0x100 and `sp` hold 0x500.
      - If you push and pop (perform stack operations) ss register will be used automatically.
      - When we pop the value on the stack into register `bx`:
        - `pop bx`
      - The address of the value we pop is:
        - 0x100 * 16 + 0x500 = 0x1500

      - So the address of the memory actually reference to is `0x1500.`

  - General purpose registers:
    - There are registers of different sizes:
      - 8 bit - al, ah, bh, bl
      - 16 bit - ax, bx, cx, dx
      - 32 bit - eax, ebx, ecx, edx
      - 64 bit - rax, rbx, rcx, rdx  (Are not available in real mode)

- Start our program:

      ```asm

      [BITS 16]
      [ORG 0x7C00]

      start:
        # 1. Clear segment registers.
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov ss, ax
  
        # 2. stack pointer start at address 0x7c00.
        mov sp, 0x7C00
      ```

- The `BITS` directive tell the assembler that our boot code is running on the 16-bit mode.
- The `ORG` directive indicates that the code is supported to be running at memory address 0x7C00.
  - The BIOS loads the boot code from the first sector into memory address 0x7c00, and transfer control to it.
  - If we don't specify 0x07C00 in this case, the memory reference within our code will be incorrect when the program is running.

- Below the `start` label, we first initialize the segment registers and stack pointer. The memory map look like:
      |---------|   /\
      |Boot code|   ||
      |---------| 0x7c00
      | Stack   |   ||
      |         |   \/
      |---------| 0

  - Because stack grows downwards, when we push a value into stack. The stack pointer will point to 2 bytes below 0x7c00 because the default operand size for push and pop instructions is 2 bytes in 16 bit mode.

- Print characters is done by calling BIOS service. The BIOS service can be accessed via the BIOS interrupts:

      ```asm
      PrintMessage:
        mov ah, 0x13
        mov al, 1
        mov bx, 0xA
        xor dx, dx
        mov bp, Message
        mov cx, MessageLen
        int 0x10

      Message:      db "Hello world"
      MessageLen:   equ $-Message

      ```

  - We the `int` instruction and then the interrupt number to call the specific BIOS service.
  - Before we call a BIOS function, we need to pass parameters. Print function has relatively large set of parameters for different purposes:
    - `ah` holds the function code, 0x13 means print strings.
    - `al` specifies the write mode, we set to 1 meaning that the cursor will be placed at the end of the string.
    - `bx`  with `bh` is the higher part of `bx` register represents page number. `bl` the lower part of `bx` holds the information of character attributes. 0xA means the character is printed in bright green.
    - `dx` with `dh` represents rows and `dl` represents columns. We set to 0 to send characters at begin of the screen.
    - `bp` hold the address of string.
    - `cx` specifies the number of characters you want to print.
    - NOTE: if you want to copy data from Message to bp we use `mov bp, [Message]`, The Message label will return the address of string.
  - `$` means the current address, by minus it to Message, we have the length of message.

- Stop program:

      ```asm
      End:
        hlt
        jmp End
      ```
  
  - The `hlt` halt instruction places the processor in a halt state.

- The partition entries + boot sector signature:

      ```asm
        times (0x1BE - ($-$$)) db
        db 0x80
        db 0, 2, 0
        db 0xF0
        db 0xFF, 0xFF, 0xFF
        dd 1
        dd (20 * 16 * 63 - 1)

        times (16 * 3) db 0
        db 0x55
        db 0xAA
      ```

  - Directive `times` repeats command the specific times. `times (0x1BE - ($-$$)) db` expression specifies how many times `db` is repeated. The `$$` means beginning of the current section (In this case, we have only one boot section 512 bytes).
    - `($-$$)` means size from start of the code to current code (the instruction before the times command).
    - Memory map look like:

      |---------------| $$ (start of the code)
      |               |
      |---------------| $ (current instruction, end of the message)
      |This region is |
      |filled with 0  |
      |---------------| 0x1BE

    - Result of the times instruction, we can clear memory space.
  - The offset 0x1BE is called partition entries. There are four entries with each entry being 16 bytes in size. This is first entry with it format:

        ```assembly
        db 0x80                 # Boot indicator
        db 0, 2, 0              # Starting CHS
        db 0xF0                 # Type
        db 0xFF, 0xFF, 0xFF     # Ending CHS
        dd 1                    # Starting sector
        dd (20 * 16 * 63 - 1)   # Size
        ```
    - First byte is boot indicator, with 0x80 means this is a boot-able partition. Next three bytes construct a starting C, H, S value.
      - C stands for cylinder, H for head and s for sector.
      - First byte is Head value.
      - Second byte [0:5] sector value, [6:7] is a cylinder value.
      - Last byte holds the lower 8 bits of cylinder value.
      - The cylinder and head values start from 0, whereas sector values start from 1 (meaning that sector 1 is the first sector)

      - We set ending of C, H, S value to 0xFF means max value we can set in a byte.
      - Next double word represents LBA address (Logic Block Address) of starting sector. In the boot process, we will load our file using LBA instead of C, H, S value.
      - The last double word specifies how many sectors the partition has We set it to `(20 * 16 * 63 - 1)` means 10MB.

    - And other entries are simply set to 0 with `times (16 * 3) db 0`.
  - We need to define these entries because some BIOS will try to find the valid looking partition entries. If don't have these entry, the BIOS will treat USB flash drive as a floppy disk.
  - We want BIOS to boot the USB flash drive as hard disk, so we add the partition entry to construct a seemingly valid entries.

  - The last two byte of our boot file is signature which should be 0x55AA.

- The size of boot file is 512 bytes. And the sector size is assumed to be 512 bytes in this course. So when we write the boot file into the boot sector, it gets the same data as boot file.

### 13. Test Disk Extension Service

- First of all, we need to test the Disk Extension Service, because we need to our kernel from disk to the memory and jump to the kernel.

- We use BIOS disk services to load our file from disk in boot process.
- When we read file from disk, we should provide CHS value in order to locate the sector we want to read. And using the CHS value requires extra calculation. To make our boot file simple, we choose the Logical Block Address (LBA) which disk extension service used to access the disk.

- Modern computers should support the extension service, but we will check it anyway.

      ```assembly
      TestDiskExtension:
        mov [DriveID], dl
        mov ah, 0x41
        mov bx, 0x55AA
        int 0x13          ; call INT 13 Extensions - INSTALLATION CHECK service.
        jc NotSupport     ; CF - Carry Flag is set on error or clear on success.
        cmp bx, 0xAA55    ; bx = 0xAA55 if installed.
        jne NotSupport

      DriveID:    db 0
      ```
  - TO check Extension use `int 0x13` we should pass:
    - NOTE: About the int 0x13 extensions [link](http://www.ctyme.com/intr/rb-0706.htm)
    - `ah` = 0x41
    - `bx` = 0x55AA
    - `dl` holds the driver ID, when BIOS transfer control to our boot code.
    - This returns:
      - `CF` flag is set on error (extensions not support)
      - `CF` flag is clear if successfully.
  - Because we want to write the value into the memory location the `DriveID` represents, we use square brackets to access the location [].
  - If the service is not supported, the carry flag is set. So we use `jc` instruction.

### 14. Loader

- After disk extension check is passed, we load a new file called loader file in the memory.
- The reason we need a loader file is that the MBR is fixed size which is 512 bytes. There are spaces reserved for other use such as partition entries which leaves us less than 512 bytes for the boot code.
- And the tasks that we should do in the boot process includes load kernel file, get memory map, switch to protected mode and then jump to long mode. Doing all these tasks require the boot code larger than 512 bytes.

- We need to a new file: loader file to do all these things.
- The memory map look like:

              Memory
      |-------------------| Max size
      |                   |
      |-------------------|
      |      Loader       | 0x7e00
      |-------------------|
      |     MBR code      | 0x7c00
      |-------------------|
      |                   |
      |                   |
      |-------------------| 0

- The loader file has no 512 bytes limits.
- The boot code is loaded by BIOS in the memory address 0x7c00. The size of the boot code is 512 = 0x200 bytes, so we simply load our loader file into the location right after the boot code which is the location 0x7e00.

- The loader do:
  - Loader retrieves information about hardware.
  - Prepare for 64-bit mode and switch to it.
  - Loader loads kernel in main memory.
  - Jump to kernel.

- our load loader code:
      ```assembly
        ; 4. Load the loader.
      LoadLoader:
        mov si, ReadPacket        ; Load the packet address to si.
        mov word[si], 0x10        ; Packet size is 16 bytes.
        mov word[si + 2], 0x05    ; We we load 5 sectors which is enough space for
                                  ; our loader.
        mov word[si + 4], 0x7E00  ; Offset which we want to read loader file.
        mov word[si + 6], 0x00    ; Segment, the logical memory to load the file
                                  ; is: 0x00 * 0x10 + 0x7E00 = 0x7E00
        mov dword[si + 8], 0x01   ; 32 bit low address of LBA.
        mov dword[si + 12], 0x00  ; 32 bit high address of LBA.
                                  ; We will start at sector 2 but set 1 to LBA
                                  ; Because the LBA is zero-based address.

        mov dl, [DriveID]         ; DriveID param.
        mov ah, 0x42              ; Use INT 13 Extensions - EXTENDED READ service.
        int 0x13                  ; Call the Disk Service.
        jc ReadError              ; Carry flag will be set if error.

        ; 5. Loader code has been loaded to physical memory, jump to loader code and 
        ; transfer control to it.
        mov dl, [DriveID]
        jmp 0x7E00
      ```

- To load the loader file the disk service we use is interrupt 0x13. with function code:
  - `ah` is 0x42: We will use disk extension service: IBM/MS INT 13 Extensions - EXTENDED READ
  - `dl` hold the driveID.
  - `ds:si` hold `disk address packet`, this actually is a structure.
    - Format of this structure:

      |offset |size |Description                                    |
      |0x00   |BYTE |Size of packet (0x10 or 0x18)                  |
      |0x01   |BYTE |Reserved (0)                                   |
      |0x02   |WORD |Number of blocks (sectors) to transfer         |
      |0x04   |DWORD|-> transfer buffer (0x04-offset, 0x06-segment) |
      |0x08   |QWORD|starting absolute block number                 |
      |0x10   |QWORD|64 bit flat address of transfer buffer;        |
      |       |     |used if DWORD at 0x04 is 0xFFFF:0xFFFF         |

    - The size of the structure is 16 bytes.
  - The last two words are the 64-bit logical block address. The loader file will be written into the second sector of the disk, Therefore, we use LBA is 1.
    - Remember logical block address is zero-based address. Meaning that the first sector is sector 0 (boot sector), and the second sector is sector 1 and so on.

- When we successfully read the loader file into memory, the next thing we are going to do is we are going to jump to the start of loader.

### 15. Check long mode is supported

- After load the loader file, the loader code will be run, we need to check if the CPU support long mode or not. Modern processors should support long mode. And we need make another check 1GB page support or not also.

- To do make checks, we will use an special instruction `cpuid` which returns processor identification and feature information. By pass different numbers to `eax` we will get different information.
  - We pass the `0x80000001` to `eax` means we get the information `Extended Processor Signature and Extended Feature Bits.`
  - But before we get the information, we need to check if the CPU is provided it or not.

        ```assembly
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
        ```

### 16. Load kernel file

- The main purpose of the loader is load kernel. So we need to create a new file kernel.asm which contains the kernel code.

- The memory map until now:

              Memory
      |-------------------| Max size
      |      Free         | -> we will use this region for kernel code.
      |-------------------|0x00100000
      |      Reserved     |
      |-------------------| 0x80000
      |      Free         |
      |-------------------|
      |      Loader       | 0x7e00
      |-------------------|
      |     MBR code      | 0x7c00
      |-------------------|
      |      Free         | -> We used this region for stack.
      |-------------------|
      | BIOS data vectors |
      |-------------------| 0

- The memory map is different depends on the BIOS and CPU version, so we need to check if the memory is available or not (Some region can be spent for hardware, for example.), this is done by calling the BIOS service.

- We will choose to load the kernel at address 0x100000, we will check it is available before load the kernel file to it.

- And we want to load 100 sectors of data roughly 50 kilobytes which is enough for our kernel.

        ```assembly
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
        ```

- So from now, the kernel is actually on address 0x10000.

### 17. Get memory map

- We use system map service to get memory blocks. After we get the free memory information, the memory module will use the information to allocate memory.

        ```assembly
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
        ```

- We will get all memory map (physical memory) from 0 to the end.

### 18. Test A20 line

- The A20 line is introduced in the old machines, back in the day when machines had 20-bit address, we can address memory of 2^20 which is 1MB.
- But later machines come with address bus wider than 20 bits. For compatibility purposes, A20 line of the address bus is OFF. So when the address is higher than 1MB, the address will get truncated as if it starts from 0 again.

- Our system runs on the 64-bit processor which has address wider than 20 bits. If we try to access memory with A20 line disabled, we will end up only accessing even megabytes because A20 line bit is zero. And any address we are gonna access is actually the address with bit 20 cleared.

- Therefore we need to toggle A20 line to access all the memory.

- There are different ways to activate A20 line and each method is not guaranteed to be working across all the different machines. But newer machines seem to have A20 line enabled by default.

- We assume our machines enabled A20 line. But we still need to check if it is enabled or not.

- To test A20 line is very simple. We will try to access memory at address 0x107C00 , if A20 is not enabled, the value we get will be from address 0x07C00.
  - If enable:      0001 0000 0111 1100 0000 0000
  - If disable:     0000 0000 0111 1100 0000 0000

  - IF A20 line is off, then bit 20th will be 0.
  - We choose this address because that is start address of boot code and the boot code has done its work, we reuse that memory to test.

- Our test A20 line assembly code:

        ```assembly
            ; 6. Check if A20 line is enabled on machine or not.
        TestA20lLine:
            mov ax, 0xFFFF
            mov es, ax                      ; Set extra segment to 0xFFFF
            mov word[ds:0x7C00], 0xA200     ; Set 0xA200 to address 0x7C00.
            cmp word[es:0x7C10], 0xA200     ; compare value at address 0x107C00 with
                                            ; 0xA200.
            jne SetA20LineDone              ; If not equal, means previous instruction
                                            ; actually reference to address 0x107C00.
            mov word[0x7C00], 0xB200        ; If equal, will make a test with different
            cmp word[es:0x7C10], 0xB200     ; value to confirm.
            je NotSupport                   ; If it actually disable A20 line we exit.

        ```

### 19. Set Video mode

- Before we switch to long mode, one more thing we need to do is set video mode. Because when we switch, we will not able to call BIOS function anymore.

- So, after we setup the text, we can print message on screen without calling BIOS service. If we pass 0x3 to `al`:
  - The base address for text mode will be 0xB8000.
  - The size of screen we can print on is 80 * 25.

- Every character takes up two bytes of space.

- The first position on the screen corresponds to the two bytes at 0xB8000, and so on.
  - The first byte is for ASCII code and second byte is attribute of character.
    - lower part of second byte is foreground cloud.
    - the higher half is background cloud.

            ```assembly
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
            ```

- Values for video mode ([BIOS video mode](http://www.ctyme.com/intr/rb-0069.htm)):

    text/ text pixel   pixel   colors disply scrn  system
    grph resol  box  resolution       pages  addr
    03h = T  80x25  8x8   640x200   16       4   B800 CGA,PCjr,Tandy

### 20. Protected mode

- What do we do to switch to protected mode?
  - Loading GDT Global Descriptor Table and IDT Interrupt Descriptor Table.
  - Enable Protected Mode.
  - Jump to Protected Mode Entry.

- **Global Descriptor Table** is a structure in memory created by us and is used by CPU to protect the memory.
    |       NULL      |
    |    Descriptor   |
    |    Descriptor   |
    |    Descriptor   |
    |    Descriptor   |
    |    Descriptor   |
    |        ...      |

  - Each table entry here describes a block of memory or segment and it takes up 8 bytes. The first entry should be entry.

  - We can have more than 8000 entries. But we only need at most 5 entries for our OS.
  - When the processor accesses memory, the value in segment register is actually an index within the GDT.
    - NOTE: This is different than we saw in real mode where the value in segment register gets shifted 4 bits and added to the offset.
  - In PM, segment registers hold the **index** instead of base address.

- For example, in protected mode, we run:
  - `mov eax, [ds:0x1000]`: this instruction will copy the value in `ds` 0x1000 to register `eax`.
  - Suppose `ds` holds value 16.
  - Remember each table entry takes up 8 bytes. So `16 here points to the third entry.
    - The segment registers also hold other information in the lower 3 bits.
  - The descriptor entry includes base address of the segment, segment limit and segment attributes.
  - Suppose that the third entry have base address are 0. So The base of this segment we are about to access is 0. Then we add the offset to the base which produces the address 0x1000.
  - The descriptor entry includes the segment attributes. So if we don't have right to access the memory, this operation will fall and the exception is generated.
  - If all the checks are done, we can access the data in that memory location.

- So when running in protected mode, setting up different segments with different privilege levels, we can protect the essential data from accessing by user applications.

- **Interrupt Descriptor Table**.
  - Basically, an interrupt is a signal sent from a device to CPU and if CPU accept the interrupt, it will stop the current task and process the interrupt event.
  - Different numbers are assigned to different interrupts. For example, when we press a key, the interrupt request number we will get is 1.
  - And also, a handler or interrupt service routine is used so that cpu will call that handler to do the specific task when an interrupt is fired.
  - In order to find that handler, we use **interrupt descriptor table** which could have a total of 256 entries.
  - This table would be a structure in memory created by us similar GDT. But the fields of IDT entries are different from those of GDT.

- For example:
  - suppose the interrupt request number 3 is fired, CPU will get the corresponding item in the IDT.
  - In the IDT entry, we have the information about which segment the interrupt service routine is at and the offset of the routine.
  - In this example, the base address of the segment this handler locates at is 0 and the offset is 0x5000.
  - Then we get the address of the interrupt handler and the code of the handler is executed.

- Privilege rings or privilege levels.
  - We have 4 rings from ring 0 to ring 3 in x86 architecture.
  - The ring 0 id the most privileged level where we can execute all the instructions and access all the hardware.
  - The ring 3 is the least privileged level where we can only execute a subset of instructions and accessing hardware is generally not allowed.
  - Although CPU offers 4 levels of privileges, we only use 2 of them in our OS.
    - Kernel run in ring 0.
    - User applications run in ring 3.
  - How do we know what level we are currently running at?
    - The CPL Current Privilege level will tell us. It's stored in the lower 2 bits of `cs` and `ss` segment registers.
      - When the 2 bits is zero means we are in ring 0. If they are 3, means we are running in ring 3.

  - In protected mode, the CPL and RPL (Requested Privilege Level) will compare against DPL (Descriptor Privilege Level), if the check fails the CPU is generated.
    - DPL is stored in Descriptor Table entries, and RPL is stored in the selectors.

  - Consider follow example:
     15           3  2  1       0
    |Selector Index|TI |   RPL   |
    - The segment selector, as its name implies, holds the index of GDT or LDT (Local Descriptor Table) entries.
    - TI means Table Indicator.
    - When TI = 0 the GDT is used and when TI = 1 the LDT is used.
      - We always set it to 0. Since we don't use LDT in our system. RPL is stored in the lower 2 bits.
    - Suppose we load the selector which points to the third entry of GDT in ds register, the privilege checks is performed.
    - The CPL is stored in `cs` and `ss` register, RPL specified in this selector are compared with DPL in the third GDT entry. If the CPL and RPL is numerically smaller or equal to DPL, the privilege checks pass.

- Descriptor Entry:
  - Code segment.
  - Data segment.

- General structure of data and code segment descriptors:
 63        56         52              48         40          16               0
|Base [31:21]|Attributes| Limit [19:16] |Attributes|Base [23:0]| Limit [15:0] |

  - We have base address of the segment, and the limit which specifies the size of the segment.
  - The attribute includes DPL field, writable, accessed bits etc.
  - For example, if the DPL of the descriptor is 0, then we are not allowed this descriptor in the segment registers if we are running in ring 3.

- We will not going to detail about descriptor, because the **segmentation is disabled in 64 bit mode**. For example, the base and limits of descriptors are ignored. The memory like flat memory starting from 0 and spanning all of the memory. And also, the segment register we use in 64-bit mode is only `cs` register. We don't care about `ds` and `es` registers. `ss` register is only used in context switch.

- But we also need to know about the general idea of what GDT, IDT and selectors are and how they interact with each other.

- Let enable the the protected mode, first, we need to disable all interrupt so that processor will not respond to the interrupt. We will re-enable them when we switch to long mode.

- After disable interrupts, we load the GDT, IDT structure, them us stored in memory and we need to tell the processor where they are located. There is a register called Global Descriptor Table Register which points to the location of the GDT in memory and we load the register with the address of gdt and the size of GDT.

- The bits [40-47] in data or code segment descriptor have structure:
     7 6 5 4 3  0
    |P|DPL|S|TYPE|
  - The s means the segment descriptor is a system descriptor or not. We need to set it to 1 meaning that it's a code or data segment descriptor.
  - The type field is assigned to the value 0010 or 1010 in binary, we assign 1010 means that the segment is non-conforming readable code segment.
    - The major difference between conforming and non-conforming code segment is that the CPL is not changed when the control is transferred to higher privilege conforming code segment from lower one. Conforming code segment in not used in our system. As for non-conforming code.
  - The DPL indicates privilege level of the segment. We will set 0x00, when we run in code segment. So when we load the descriptor to `cs` register, the CPL will be 0 indicating that we are at ring 0.
  - P - present bit which means we need to set it to 1 when we load the descriptor.

- Next byte us a combination of segment size and attributes bits [48-55]:
     7 6 6 4 3   0
    |G|D|O|A|LIMIT|

  - The lower half is the upper 4 bits of the segment size. we will set up to maximum size.
  - A - The Available bit can be used by the system software, we will ignore it by set 0.
  - O - Operation bit, if 1 means that the default operand size is 0. Otherwise, the default operand size is 16 bits. We will set to 1 in PM.
  - G - Granularity bit. We simply set it to 1 meaning that the size field is scaled by 4KB. Which gives us maximum of 4GB of segment size.

- The last byte of code segment is upper 8 bits of base address. We simply set it to 0.
- So the code segment descriptor look like:

        ```assembly
        CodeSegDes:         ; Next entry is Code Segment Descriptor.
            dw 0xFFFF       ; First two byte is segment size, we set to maximum size for
                            ; code segment.
            db 0, 0, 0      ; Next three byte are the lower 24 bits of base address, we
                            ; set to 0, means the code segment starts from 0.
            db 0b10011010   ; Next byte specifies the segment attributes, we will set
                            ; code segment attributes: P=1, DPL=00, S=1, TYPE=1010.
            db 0b11001111   ; Next byte is segment size and attributes, we will set code
                            ; segment attributes and size: G=1,D=1,O=0,A=0,LIMIT=1111.
            db 0            ; Last byte is higher 24 bits of bit address, we set to 0,
                            ; means the code segment starts from 0.
        ```

- Next, we need define segment descriptor for data segment, these descriptors are very similar (base address and size), different is type field. We set it to 0010 in binary means the segment is readable and writable.

- After define GDT we need to load it, first we define GDT32 pointer to load:

        ```assembly
        GDT32Len: equ $-GDT32

        GDT32Pointer: dw GDT32Len - 1   ; First two bytes is GDT length.
                      dd GDT32          ; Second is GDT32 address.
        ```

- Load it with `lgdt` instruction:
  - `lgdt [GDT32Pointer]     ; Load global descriptor table.`
  - Note that the default operand size of `lgdt` instruction is 16 bits in 16-bit mode. If the operand size is 16 bits, the address of GDT pointer is actually 24 bits. But here we simply define the address to be 32 bits. And assign the address of GDT to the lower 24 bits.

- Next thing we are going to do is load IDT, just like global descriptor table register, we also have interrupt descriptor table register and we need to load the register with the address and size of IDT, we will use `lidt` instruction.
  - Since we don't want to deal with interrupts until we jump to long mode, we load the register with Invalid address and size zero.

        ```assembly
            lidt [IDT32Pointer]     ; Load an invalid IDT (NULL) because we don't deal
                                    ; with interrupt until switching to Long Mode.

        IDT32Pointer: dw 0              ; First two bytes is IDT length.
                    dd 0              ; Second is IDT32 address.
        ```
  - Note that there is one type of interrupt called non-maskable interrupt which is not disabled by the instruction `cli`. So non-maskable interrupt occurs, the processor will find in memory, if it is invalid address the CPU exception will be generated, our system will be reset.
    - Non-maskable interrupt indicate that non-recoverable hardware errors such as RAM error, for example.

- After load GDT and IDT, we enable protected mode by setting the Protected Mode Enable bit[0] in `cr0` register (Control register), which register changes or controls behavior of the processor.

        ```assembly
            mov eax, cr0            ; We enable Protected Mode by set bit 0 of Control
            or eax, 0x01            ; register, that will change the processor behavior.
            mov cr0, eax
        ```

- Last thing we need to do is load the `cs` segment register with the new code segment descriptor, we just define in the gdt table. Loading code segment descriptor to `cs` register is different from another segment registers. We can not using `mov` to do this, instead we jump instruction to do it.

- The code segment descriptor in this example is the second entry which is 8 bytes away from the beginning of the GDT. So the selector we use is index being 8, and T=0 meaning that we use GDT and the RPL is 0. Therefore when CPU performs privilege checks, the RPL is 0 and is equal to the DPL of code segment, the check will pass. So we simply specify the offset 8, we want to jump to the protected mode entry which is label we will define.

- Before we define the label, we use directive `bits` to indicate that the following code is running at 32-bit mode.

        ```assembly

        jmp 0x08:ProtectedModeEntry

        [BITS 32]
        ProtectedModeEntry:
        ```

- After entry the PM, we need to initialize other segment registers (`ds`, `es`, `ss` registers):
  - We load them with data segment descriptors. The data segment descriptor is the third entry 0x08 + 0x98 = 0x10
- And also set the `sp` to 0x7C00
        ```assembly

        jmp 0x08:ProtectedModeEntry

        [BITS 32]
        ProtectedModeEntry:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov esp, 0xc7C00
        ```

- That is done for protected mode, we can print something.

### 21. Long Mode

- Long mode consists of two sub modes 64-bit mode and compatibility mode (we not implement this mode).
- So, our kernel and applications run in 64-bit mode. When we jump to kernel entry will stay there and never leave this mode until we shutdown the system.

- Protected mode is only used to prepare for long mode. Immediately after jumping to protected mode, we prepare and switch to long mode which is a little bit complex, than preparing for protected mode.

- Switch to long mode require some steps:
  - 1. Load GDT
  - 2. Load IDT
  - 3. Prepare for paging and enable it
  - 4. Enable long mode.
  - 5. Jump to long mode entry.

- NOTE: we have to enable paging before entering long mode, since segmentation is disabled in 64-bit mode. We will use paging to protect memory instead.
- We can enable long mode by using the specific registers and load the new code segment descriptor in `cs` register using jump instruction.

- GDT is still used in 64-bit mode and the field of GDT entries are slightly different. Regarding to segment registers, `cs` register is used, however the contents of `ds`, `es`, `ss` registers are all ignored. The contents in hidden parts of those registers are also ignored. The memory segment referenced by those segment registers are treated as if the base address is 0, and the size of the segment is the maximum size the processor can address. Actually CPU doesn't perform segment limit checks as we saw in PM. So generally we don't need data segment descriptors.
- But there is one special case where we need the data segment descriptors, that is, when we try to get from ring 0 to ring 3 to run user applications. Because getting from ring 0 to ring 3 requires us to add a valid data segment descriptor to load into `ss` register.

- So the code and data segment will be still used.

- The content of `ds`, `es`, `ss` registers are all ignored. But cpu still performs checks to see if the descriptor is valid. If it is an invalid descriptor, cpu will emit an exception.

- For example: `mov eax, [ds:0x1000]`
- `ds` always is ignored and the base address is always 0.

- The IDT is still used in 64-bit mode. Each entry takes up 16 bytes, whereas in PM is 8 bytes.

- The privilege levels are the same with PM.

- If we want to load a descriptor into segment register, the index should point to valid descriptor. And CPL, RPL will compare against DPL in the sector.
- In 64 bit mode, it's really rare to load segment descriptors by ourselves. In our system, we load code segment descriptors when we jump to 64 bit mode and load data segment descriptor in `ss` register.

- Descriptor Entry in long mode:
  - Long mode ignore almost field in 8 bytes of descriptor entries which leaves us only few bits in attributes field to set.
    - Bit[42] - C: conforming bit: we only use non-conforming code segment.
    - Bit[43] - 1: we set bit [43:44] to 11 indicate that descriptor is code segment descriptor.
    - Bit[44] - 1
    - Bit[45:46] - DPL : the privilege level we will set it to 0.
    - Bit[47] - P - Present bit should be 1.
    - Bit[53] - L - Long bit, which is new attribute bit, set it to 1 means we are running on 64 bit mode. If 0, we are run on compatibility mode.
    - Bit[54] - D - only valid value is 0 when we set long bit = 0.

- The data segment descriptor also has very few bits we can set.
  - Bit[41] - W - Writable, we will set to 1.
  - Bit[43] - 0 - bit[43:44] = 10 mean the descriptor is data segment descriptor.
  - Bit[44] - 1
  - Bit[45] - DPL - privilege bit should be 0.
  - Bit[46] - P - present bit should be 1.

- There are two terms **Virtual Address** and **Physical Address**. The virtual address is basically we in our code and **these addresses do not directly go to the memory bus**.
  - Virtual addresses need to be mapped to physical addresses by using Memory Management Unit or MMU.
  - The physical address is the address put on the memory bus and the data in that memory location is accessed.
  - There are only few cases where we use physical address in our code directly.
  - In 64 bit mode, we have 64 bits of virtual address space. But not all the virtual addresses are available to us.

- The canonical address is the address with the bits 63 through to the Most-Significant implemented bit are either all ones or all zeros. For example, suppose we have a 48 bit address here from bit[0:47]. So we can only set the upper 16 bits which is from bit 48 to bit 63 to all ones.

- Generally, when we use non-canonical address, the CPU exception is generated. Also, note that Intel has added support for 57-bit virtual address in recent processors which are able to address 128petabyte memory space. IN our OS, we assumption that we only support 48 bit virtual address, so that our system can run on older machines.

- Memory map setup for 64 bit mode:

              Memory
      |   Kernel space    |
      |                   | 0xFFFF800000000000
      |-------------------|
      |                   | 0xFFFF7FFFFFFFFFFF
      |   Non-canonical   |
      |     Addresses     | 0x800000000000
      |-------------------|
      |    User space     | 0x7FFFFFFFFFFF
      |                   |
      |-------------------| 0

- The user space and kernel space are the address space available to us. The `Non-canonical Addresses` region are the address range where all the addresses are Non-canonical address which cannot be used. You can check out the bit 47 and the upper 16 bits of these address yourself and you will find out they doesn't. conform to the canonical address.

- And our kernel will be placed at the higher part of the memory space.

- First thing we need to do in PM is enable paging, enable long mode requires us to setup and enable paging.

        ```assembly
            ; 11. Enable paging.
            ; This bock of code basically finds a free memory area and initialize the
            ; paging structure which is used to translate the virtual address to 
            ; physical. The addresses (0x80000 to 0x90000) may used for BIOS data. We 
            ; can use memory area from 0x70000 to 0x80000.
            ; TODO: add detailed document about this block.
            cld
            mov edi, 0x80000
            xor eax, eax
            mov ecx, 0x10000/4
            rep stosd

            mov dword[0x80000], 0x81007
            mov dword[0x81000], 0b10000111
        ```

- Then we load the GDT that we will use in 64-bit mode. Just like we did in protected mode.

        ```assembly
        ; Global Descriptor Table Structure for 64 bit mode, we define 3 entries with 8
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
        ```

- After load GDT, we will enable 64 bit mode by set necessary bits in some registers.
  - In `cr4` bit cr4[5] is called Physical Address Extension or PAE bit. We have to set it to 1 before activating 64 -bit mode.

- The next register is related to `cr3` register, we copy the address of page structure we just set up 0x80000 t0 `cr3` register.
- The address loaded to `cr3` are physical address.

- After we enable paging and enter long mode, the addresses in our code need to be mapped to physical addresses first and then sent to the bus. But if the we load new address in `cr3` register, we still need physical address instead of virtual address.

- Then we can enable long mode. There is a model specific register called Extended Feature Enable Register, adn the bit 8 is long mode enable bit which should be set. To read and write a model specific register, we move the index of the register to `ecx`: `0xC0000080` in this case and execute read MSR `rdmsr` instruction. The return value is in `eax` register. So we set the bit 8 in eax using or instruction. Then write it back to the same register by using writer msr `wrmsr`.
- Last thing we will do is enable paging by setting the bit 31 in register `cr0`.

        ```assembly
            ; 13. Enable 64-bit mode.
            mov eax, cr4            ; Set bit 5 Physical Address Extension of Control
            or eax, (1<<5)          ; register 4.
            mov cr4, eax

            mov eax, 0x80000        ; Setup the page structure we just setup 0x80000 to
            mov cr3, eax            ; cr3 Control Register 3. By the way, all the
                                    ; addresses we have seen so far are all physical
                                    ; addresses because we don't enable paging.

            mov ecx, 0xC0000080     ; Enable Long mode, using Extended Feature Enable
            rdmsr                   ; Register, it is a special register. We need to get
            or eax, (1 << 8)        ; it via index (0xC0000080) pass to ecx register,
            wrmsr                   ; and then call read MSR instruction, result will
                                    ; return to eax. So will set bit[8] in eax to enable
                                    ; long mode and write back to the MSR register.
            
            mov eax, cr0            ; Enable Paging by set bit[31] on cr0 Control
            or eax, (1 << 31)       ; Register 0.
            mov cr0, eax
        ```

- The Long mode activated from Now. We need to jump to long mode entry to load the new code segment descriptor. We specify the segment selector 8, since each entry is 8 bytes and the code segment selector is the second entry and then the offset long mode entry.

        ```assembly
            jmp 0x08:LMEntry
        ```

- After jump to long mode code, we don't need initialize `ds`, `es`, `ss` because they are ignored. But will still need to initialize stack to 0x7C00.

## V. Exceptions and Interrupts Handling on the x86

### 22. Jumping to Kernel

- The kernel file is loaded into memory and are going to jump to the kernel. We relocate the kernel from 0x10000 to the memory 0x200000.

- `cld` instruction clear direction flag so the move instruction will process the data from low memory address to high memory address. Which means the data is copied in forward direction. The destination address is stored in `rdi` register and source address is in `rsi` register.

- Register `rcx` acts as a counter, since we want the move instruction to execute multiple times, move q-word will copy the 8 bytes data each time.
  - We will move 51200 / 8 bytes to `rcx`. Because our kernel size is 512 sectors = 512 * 100 = 51200 bytes.

- `rep movsq` repeat by quad-word.
- After instruction, our kernel is copied into the address 0x200000.

### 23. Reload GDT

- The first thing we will do in the kernel is manage the system resources in one place such as GDT and IDT.

- Notice that the address of GDT in GDT pointer is 8 bytes (quad-word). In 64-bit mode, we load the pointer with 2 byte limit and 8 byte base address. So we need to change the double word to quad-word.

- And then, we are going to load the code segment descriptor into `cs` register. instead using jump instruction, we load descriptor using the return instruction.
  - `retf` indicate that this is a far return. Normal return will not load the descriptor in `cs` register.
  - When we execute the return instruction, we know that we will return from current procedure to the caller. The return address is pushed on the stack by the caller and go to the procedure.
  - In the procedure, return instruction will pop the address in `rip` register which will branch to that location.

- WHat we are actually doing here is fabricate the scenario where we are called by other caller and the far return instruction executes and we are back to the caller. So the return address need to be pushed on the stack on our own and we will jump to that address.

- Generally we have two situations regarding to far return. One is return to the same privilege. Another is return to the different privilege level.

- With same privilege, we can setup top stack look like:

    |CS selector| 0x7C00
    |Offset     | <--- RSP

- The first 8 bytes are the code segment selector, here we choose 0x08. Then the offset is in the next 8 bytes. So we save the address of location we want to branch in here.

        ```assembly
            push 0x08           ; Push Code Selector.
            push KernelEntry    ; Push Kernel entry address.
            retf
        ```

- Also note that the default operand size of far return is 32 bits. Each data we pushed on the stack is 64 bits. So we need to add operand-size override prefix 48 ourselves to change the operand size to 64 bits. `db 0x48`

### 24. Exceptions and interrupt handling

- Interrupt handling is especially important part of the operating system. For example, if we don't implement interrupt handling for keyboard, then we cannot use keyboard for interact with computer.
- Process scheduling which is an essential part of our OS relies on timer interrupt.

- When we we talk about interrupts, we actually refer to hardware interrupts such as timer, keyboard interrupt, etc. Whereas exception occurs as a result of execution errors or internal processor errors. The process of handling interrupts and exceptions are very similar.

- When an interrupt is fired, the correspond IDT entry is selected, the entry includes the information about which code segment the interrupt handler is at and its attributes, the offset and other info, etc.

- The IDT entry structure:
 127    95        64  63         48         40   34 32 31        15         0
|      |offset[63:32]|offset[31:16]|Attributes| |IST  |Selector |offset[15:0]|

- The IDT entry takes up 16 bytes. The 64-bit offset of the interrupt handler is divided into three parts:
  - lower 16 bits is stored in the first two bytes.
  - second part is bits[48:63]
  - finally part is bits[64:95]

- Bit 16 to 31 holds the selector of the code segment descriptor which the interrupt handler is at.
- `IST` stands for interrupt stack table. if we assign a value to this field, when the interrupt is fired the stack pointer this field references is loaded into `rsp` register. So we don't use the IST so we will set it to 0.

                     7 6 5 4 3 2 1 0
- 8 bits attribute: |P|DPL|0|1|1|1|0|
  - The present bit should be 1.
  - `01110` means this entry is interrupt descriptor table.
  - The DPL specifies that which CPL can access this descriptor. For example, if we set DPL to 0, then we cannot access the descriptor when we run in ring 3 because the CPL is 3. This field is useful when the descriptor is for the software interrupt. We will set it to 0, because processor ignores the DPL of the descriptor of hardware interrupt and exceptions.

- There are 256 different exceptions and interrupts vectors we can use:
  - 0 - Divide by 9
  - 1 - Debug
  - 2 - NMI
  - 3 - Breakpoint
  - 4 - Overflow
  - 5 - Bound
  - 6 - Invalid opcode
  - 7 - Device N/A
  - 8 - Double faults
  - 9 - Undefined
  - 10 - Invalid TSS
  - 11 - Segment fault
  - 12 - SS fault
  - 13 - GP fault
  - 14 - Page fault
  - 15 - Reserved
  - 16 - x87 FPU
  - 17 - Alignment
  - 18 - Machine check
  - 19 - SIMD
  - 20-31 - Reserved
  - 32-255 - User defined

- The numbers here are called interrupt vectors which identifies the exceptions and interrupts.

- NOTE that the vectors from 0 to 31 are predefine by the processor. We cannot redefine them. The vectors from 32 to 255 are user defined. The vectors for hardware interrupts and software interrupts are within this range.

### 25. Saving registers

- When we jump to handlers, we should save state of CPU. We will save by using push instruction to registers to stack.

- When interrupts or exceptions occur, only some of registers are saved such as `rip` and `rsp` registers. In handler, we can use and modify registers to perform specific tasks, therefore when we return from the handler, the value of registers may have changed of CPU is not the same as it was before the interrupts occurs.
- So in order to restore the state of CPU after the interrupt handling is done, we store all the general purpose registers on the stack.

- In 64-bit mode, we have 16 general purpose registers. `rsp` is pushed on the stack when cpu calls the handler. So here we push 15 registers.

        ```assembly
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
        ```

- After we handle the interrupt, we pop the origin value in those registers and return. so when we return from the handler, the previous work CPU is doing is resumed just like the interrupt never happened.

### 26. Setting up the interrupt controller

- To deal with hardware interrupt, first thing we need to do is initialize the PIC (Programable Interrupt Controller - Manage the interrupt requests - IRQ) before we can handle the hardware interrupts.

- We will set up timer interrupt. In order to do that, we also need to initialize the programmable interval timer which will fire the interrupt periodically.

- There are two chips linked together.

  Slave                       Master
| IRQ 0 | \                 | IRQ 0 |
| IRQ 1 |  \                | IRQ 1 |
| IRQ 2 |   \       /------>| IRQ 2 |
| IRQ 3 |    \______|       | IRQ 3 | =====> CPU
| IRQ 4 |    /              | IRQ 4 |
| IRQ 5 |   /               | IRQ 5 |
| IRQ 6 |  /                | IRQ 6 |
| IRQ 7 | /                 | IRQ 7 |

- Each chip has 8 IRQ signals. The slave is linked to master via IRQ2. So actually we have a total of 15 interrupts.
- The programmable interval timer, for example, uses IRQ0 of the master chip. The keyboard uses IRQ 1 of the master chip, etc. When we press a key, the keyboard will send a signal to the interrupt controller and the controller will then send signals to the processor according to the settings we write to the controller. The processor finds the handler for that interrupt by the vector number and call that handler.

- There are three registers in each chip (`IRR`-`IMR`-`ISR`). These registers are used to service the interrupts. They are 8 bit registers with each bit representing the corresponding IRQ.
- When a device, keyboard for example, sends a signal to the chip the corresponding bit in Interrupt Request Register `IRR` will be set. If the corresponding bit in the interrupt mask register `IMR` is 0, meaning that the IRQ is not masked, the PIC will send the interrupt to the processor and set the corresponding bit in the In-Service Register (ISR).

- Generally, we could have multiple IRQs come at the same which means more than one bit in the `IRR` will be set. The PIC will choose which one should be processed first according to the priority. Normally, the IRQ0 has the highest priority and IRQ7 the lowest priority. And note that the slave is attached to the master via IRQ2. So all the IRQs on the slave have higher priority than IRQ3 of the master.

- Let's get started.

- There are three channels in the PIT (Programable interval timer) through channel 0 to channel 2. channel 1 and 2 may not exist and we don't use them in our system. So we only talk about channel 0. The PIT has four registers, one mode command register and three data registers. We set the command and data registers to make the PIT works as we expect, that is fire an interrupt periodically.
  - The mode command register has four parts in it.
    - Bit[0] mean the value PIT uses is in binary or BCD forms (set to 0 mean use binary form).
    - Bit[1:3] is operating mode: 010 mean mode 2, rate generator used for reoccurring interrupt, for example.
    - Bit[4:5] is Access mode: we want to write 16 bit value to PIT, we need to write two bytes in a row. And access mode specifies in which order the data will be written to the data register, such as low byte only, high byte only, etc. We will set the access mode to 11 means we want to write the low byte first and then high byte.
    - Bit[6:7] selecting the channel.

- Address of mode command register is 0x43. We use `out` instruction to write the value in `al` to the register.

        ```assembly
        InitializePIT:
            mov al, 0b00110100  ; Initialize PIT mode command register, FORM=0, MODE=010
                                ; , ACCESS=11, CHANNEL=00.
            out 0x43, al        ; Address of mode command register is 0x43. We use out
                                ; instruction to write the value in al to the register.
        ```

- Now we can write the settings to data register to make the PIT fire the interrupt as we expect. In fact the value we want to write to the PIT is an **interval value** which specifics when the interrupt is fired. The PIT works simply like this, we load a counter value and PIT will decrement this value at a rate of about 1.2Mega HZ which means it will decrement the value roughly 1.2 million times per second.
  - In our system, we want the interrupt fired at 100 HZ which is 100 times per second.

            ```assembly
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
            ```

- Now PIT is running now. Next thing we do is set up the interrupt controller. The PIC also has command register and data register. Each chip has its own register set.
  - The address for the command register of the master chip is 0x20 and for slave chip is 0xA0.
    - With 8 bits:
      - Bit[0:3]: Indicate that we use the last initialization command word.
      - Bit[4:7]: means that this is the initialization command followed by another three initialization command words we are about to write.

            ```assembly
            InitializePIC:
                mov al, 0b00010001  ; Initialize PIC command register bits[7:4]=0001,
                                    ; bits[3:0]=0001.
                out 0x20, al        ; Write to the command register of master chip.
                out 0xA0, al        ; Write to the command register of master chip.
            ```

- Next we write these three command words. The first one specifies the starting vector number of the first IRQ. Remember the processor has defined the first 32 vectors for its own use. So we define the vectors number from 32 to 255.
  - instead of writing the data to command register, we write it to the data register, the address of which is 0x21 for the master and 0xA1 for the slave.
  - Each chip has 8 IRQs and the first vector number of the master is 32, the second vector is 33 and so on.

- On a regular system, the slave is attached to the master via IRQ 2. If the bit 2 of the word is set, it means that the IRQ2 is used.

        ```assembly
            mov al, 0b00000100  ; Select bit 2 in master chip to connect with slave.
            out 0x21, al
            mov al, 0b00000010  ; Slave identification should be 2.
            out 0xA1, al
        ```

- Last command we set in word is selecting mode:
  - bit[0]: x86 system is used.
  - bit[1]: is automatic end of interrupt. We don't use it.
  - bit[2:3]: buffered mode. We don't use it.
  - bit[4:7]: specify the nested mode. We don't use it.

        ```assembly
            mov al, 0b00000001  ; Selecting mode: bit[0]=1 means x86 system is used.
            out 0x21, al
            out 0xA1, al
        ```

- Now the interrupt controller is working. But we have one more thing to do. Since we have a total 15 IRQs in the PIC, we only setup one device. Therefore, we mask all the IRQs except the IRQ0 of the master which PIT uses.

- Masking an interrupt is simple, all we need to do is set the corresponding bit of the IRQ and write the value to the data register.

        ```assembly
            mov al, 0b11111110  ; Masking all interrupts, except IRQ0 for the timer
            out 0x21, al        ; interrupt we used.
            mov al, 0b11111111
            out 0xA1, al
        ```

- The next thing we are going to do is we are going to set the IDT entry for the timer. The vector number of the timer is set to 32 in the PIC, so the address of the entry is the base of the idt + 32*16.

### 27. Getting to ring 3

- The CPL is stored in the lower 2 bits of `cs` and `ss` registers. We are running on ring 2, if we check the lower 2 bits of `cs` register we will set the value is 0 meaning that we are running in ring 0. What we need to to is prepare the code segment descriptor for ring 3 and load the descriptor to `cs` register.

- Once the descriptor is successfully loaded into `cs` register, we will run in ring 3. In this process, the data segment descriptor for `ss` register is needed. This is the only case where we load the data segment descriptor ourselves.

        ```assembly

        ; Global Descriptor Table Structure for 64 bit mode.
        GDT64:
            dq 0            ; First entry is NULL.
        CodeSegDes64:       ; Next entry is Code Segment Descriptor.
            dq 0x0020980000000000
            dq 0x0020F80000000000   ; DPL is ring 3, we make new code segment descriptor
                                    ; that run with privilege level 3.
            dq 0x0000F20000000000   ; And make data segment descriptor that run with
                                    ; privilege level 3 also and writable.
        ```

- We use interrupt return to jump from ring 9 to ring 3. To return to the ring 3, we have to prepare 5 8-byte data on the stack. Stack frame that is saved automatically look like:

    High address    |ss selector|
                    |   RSP     |
                    |   Rflags  |
                    |cs selector|
    Low address     |   RIP     |<---- RSP

- The top of stack is RIP value which specifies where we will return. The next data is the code segment selector we will load into `cs` register after we return. `Rflags` contains the status of the CPU. When we return, the value will be loaded in `Rflags` register. The stack pointer is stored in the next location which will be loaded in register RSP. The last one is stack segment selector. Because stack is last in first out structure, we push the data in reverse order.

- So we push the stack frame and using `iretq` instruction:

        ```assembly
        push 18|3       ; ss selector
        push 0x7C00     ; RSP
        push 0x2        ; Rflags
        push 0x10|3     ; cs selector
        push UserEntry  ; push RIP
        ```

- So when CPU catch the `iretq` instruction, it automatically pop stack frame to restore the old state, that state will want.

- So we can define `UserEntry` label that run on ring 3.

### 28. Task state segment, interrupt handling in ring 3

- When we are running in ring 3, an interrupt is fired the control will be transferred to the interrupt handler which runs in ring 0 in our system. The DPL of the segment descriptor which the interrupt handler is at is set to 0. In this process, the `ss` segment register is also loaded with NULL descriptor by the processor. And the `rsp` register is also loaded with a new value. The value of `rsp` is stored in the **Task State Segment**.

- Therefore, what we need to do is set up the TSS and specify the new value of `rsp` in the TSS.
- Structure of TSS:
                            Offset
    |IOPB address| Reserved | 0x64
    |       Reserved        | 0x5C
    |          IST7         |
    |          IST6         |
    |          IST5         |
    |          IST4         |
    |          IST3         |
    |          IST2         |
    |          IST1         | 0x24
    |       Reserved        |
    |          RSP2         | 0x14
    |          RSP1         | 0xC
    |          RSP0         | 4
    |       Reserved        | 0

- The field of `RSP0` is what we want. When the control transferred from low privilege level to ring 0, the value of `RSP0` is loaded into `RSP` register. Since we don't use ring 1 and ring 2, we are not gonna set those fields. The interrupt stack table - IST, if the IST field in the interrupt descriptor is non-zero, then it is the index of IST here. For example, if it is 1, then the value of IST1 is loaded inRSP instead RSP0 value. SInce we have set the interrupt descriptors with IST field being 0, we are not gonna use the IST fields here.

- The last item is the address of io permission bitmap `IOPB address` which is used for protection for io port addresses (we do not use in our system).

- The TSS descriptor is stored in the GDT. Unlike the code segment and data segment descriptors where the base and limit are ignored, **the base and limit attributes in TSS descriptor are used**. So when we set up the descriptor, the base address and limit need to be set correctly.
 127          104        95                                                 64
|          |00000|      |                       Base [63:32]                  |

 63        56         52              48         40          16              0
|Base [31:21]|Attributes| Limit [19:16] |Attributes|Base [23:0]| Limit [15:0] |

- In the attributes[40:44] we need to set 01001 means that this is the 64-bit TSS descriptor.

- Loading the TSS selector is different from loading the code segment selector. Here we load the selector using instruction load task register. After the selector is loaded, the processor then uses the selector to locate the TSS descriptor in the GDT.

        ```assembly
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
        ```

        ```assembly
        ; Task state segment structure - 128 bytes.
        TSS:
            dd 0            ; First four bytes is reserved.
            dq 0x150000     ; Set RSP0 to new address.
            times 88 db 0   ; Clear next 88 bytes to 0.
            dd TssLen       ; IO permission bitmap, assign size of TSS means we don't
                            ; use the IO permission bitmap.
        TssLen: equ $-TSS
        ```

- Load the TSS:

        ```assembly
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
        ```

- So after load the TSS, we can test emit interrupt that run on ring 3. to do that, we will enable interrupt in `Rflags` that will be restored when we use `iretq`.

        ```assembly
            ;6. Test switch to ring 3 using iretq, we push fake stack frame.
            push 0x18|3     ; ss selector.
            push 0x7C00     ; RSP.
            push 0x202      ; Rflags, we push 0x202 to enable interrupt via state reg
                            ; when we get from ring 0 to ring 3.
            push 0x10|3     ; cs selector.
            push UserEntry  ; RIP.
        ```

- In the timer handler, we can increase some variable to check, timer is active every 100ms look like:

        ```assembly
            inc byte[0xB8004]
            mov byte[0xB8005], 0xA
        ```

- Also, when handling the hardware interrupt, we need to acknowledge the interrupt before we return from the handler. Otherwise we will not receive the interrupt again.
  - To acknowledge the interrupt, we write the value to the command register of the PIC. The bit[5] of the value is non-specific end of interrupt.

        ```assembly
            inc byte[0xB8004]
            mov byte[0xB8005], 0xA
            mov al, 0b00100000  ; Set bit 5.
            out 0x20, al        ; ACK the interrupt to run timer again, so the timer
                                ; interrupt will fire periodically.
        ```

- When the handler returns now, the timer interrupt will fire periodically and see the character is changing.

### 30. Spurious interrupt handling

- Spurious interrupt is not a real interrupt. The IRQ used for spurious interrupt is the lowest priority IRQ number that is IRQ7 for the master chip. The spurious interrupt is caused by several reasons. In order to handle the spurious interrupt, we need to implement the handler for IRQ7. Because we don't use the slave chip in our system, we only deal with the spurious interrupt in the master chip.

- We can define this handler, and perform check if it is spurious or not. To perform check, we read the ISR register of the PIC and check the bit 7 of the value. If it is set, then this is a regular interrupt and we handle it as we did with other interrupts. If is 0, this is spurious interrupt and we simply return without sending the end of interrupt. We write a value to command register:

    7 6 5 4 3 2 1 0 bit
    0 0 0 0 1 0 1 1
- The bit[0] is 1 specify reading IRR or ISR register. bit[1:0] = 11 means reading ISR register.
- Bit[3] is 1 meaning that this is the command which reads the ISR register.

        ```assembly
        ; save state.

        mov al, 0b00001011
        out 0x20, al
        in al, 0x20
        test al,(1<<7)
        jz End
        ; do some thing.

        mov al, 0x20
        out 0x20, al ; Send ack if it is regular interrupt.

        End: ; If not, return, and not enable this interrupt forever.
        ; restore state.
        ```

## 6. Working with C

### 31. Kernel Main

- The compiler we use is GCC and the executable file we will generate is elf file. Since the kernel in assembly code is also in the kernel, we will see how to combine the assembly and C files together.
- The `nasm` command will output `elf64` file for kernel.asm:

        ```bash
        nasm -f elf64 -o kernel.o kernel.asm
        ```

- Then we will link the object file with other files written in C. In the kernel assembly file, we define two sections `.text` and `.data`.
- The data section is used for the data defined globally such as IDT, TSS, etc.

- Build C file:
  - `-mcmodel=large`: Use large code model. So that code generated is for the large mode.
  - `--ffreestanding`: We don't need C standard library and other runtime features. There are still some of headers available to use usch as `stdint.h`.
  - `-fno-stack-protector`: No stack protector.
  - `-mno-red-zone`: red zone is an area of 128 bytes below the stack pointer which can be used by leaf functions without changing rsp register. The red zone is specified in system V AMD64 calling convention which we use in the code. We need to disable red zone in the kernel, otherwise the kernel stack could be corrupted if the interrupt occurs.

- We don't use the `ORG` directive to tell the assembler we want our kernel file running in the address 0x200000. But in the loader file, we still jump to the address 0x200000 after we load the kernel. So we use the linker script to do it.  We have several sections define in the files.

### 32. Process the interrupts

- We are using system V AMD64 calling convention where the first six arguments are stored in: `RDI` - `RSI` - `RDX` - `RCX` - `R8` - `R9` and others are on the stack in reserved order.
- Return value is stored in `RAX` register.
- Caller Saved Registers: `RAX`, `RCX`, `RDX`, `RSI`, `RDI`, `R8`, `R9`, `R10`, `R11`. Which means the caller has to save the value of these registers if it calls other function which could alter there registers. Normally the caller will save the value on the stack before it calls other functions and restore the value of the registers after the functions return.
- Callee Saved Registers: `RBX`, `RBP`, `R12`, `R13`, `R14`, `R15`. Which means the value of these registers are preserved when we call a function and return from it.

## 7. Memory management

### 36. Retrieve memory map

- In this section, we will build memory map manager and set up paging. After load the kernel to address 0x200000, the memory map look like:

              Memory
      |      Free         |
      |-------------------|
      |      Kernel       |
      |-------------------| 0x200000
      |      Free         |
      |-------------------| 0x100000
      |      Reserved     |
      |-------------------| 0x80000
      |      Free         |
      |-------------------|
      |      Loader       | 0x7e00
      |-------------------|
      |     MBR code      | 0x7c00
      |-------------------|
      |      Free         |
      |-------------------|
      | BIOS data vectors |
      |-------------------| 0

- Currently we have the free memory region above 1MB (0x100000) up to our kernel code (That resident start at 0x200000).
- But not all the memory map above 1MB is available to use. we need to find out memory map.

- We have retrieved the information about memory map and stored it in the address 0x9000.

        ```assembly
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
        ```

- We actually implementing the `printk()` so we can use it to print all memory map information to the screen.

- We need edit a little bit about getting memory map information, we stores the count of structures in address 0x9000, so we initialize 0x9000 with value 0 and this value is 4 byte data, And the structure are stored from address 0x9008.

        ```assembly
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
        ```

- When we retrieve memory info using BIOS service, the data is actually arranged in the structures.
