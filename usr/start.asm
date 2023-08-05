section .text
global Start
extern main

Start:
    call main
    jmp $