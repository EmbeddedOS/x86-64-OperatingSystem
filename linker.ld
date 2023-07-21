OUTPUT_FORMAT("elf64-x86-64")
ENTRY(Start)

SECTIONS
{
    . = 0x200000;
    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    . = ALIGN(16);

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}