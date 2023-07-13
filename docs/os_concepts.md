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
