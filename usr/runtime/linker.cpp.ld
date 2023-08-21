OUTPUT_FORMAT("elf64-x86-64")
ENTRY(Start)

SECTIONS
{
    . = 0x400000;
    .text : {
        *(.text)
    }

    .rodata ALIGN(0x1000) : {
        /* GNU C++ will will normally arrange to put the addresses of global
         * constructors and destructors into the `.ctors` and `.dtors` or
         * `.init_array` and `.fini_array` sections.
         */
        __constructor_array_start = .;
        *(SORT(.init_array*))
        *(SORT(.ctor*))
        __constructor_array_end = .;

        __destructor_array_start = .;
        *(SORT(.fini_array*))
        *(SORT(.dtor*))
        __destructor_array_end = .;

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