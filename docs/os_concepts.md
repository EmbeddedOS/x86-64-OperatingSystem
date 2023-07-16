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
