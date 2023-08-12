OUTPUT_FORMAT("elf64-x86-64")
ENTRY(Start)

SECTIONS
{
    . = 0xFFFF800000200000;
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

    /* We make this symbol to get the end of our kernel. */
    PROVIDE(l_kernel_end = .);
}