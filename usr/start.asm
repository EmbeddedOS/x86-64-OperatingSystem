section .text
global Start
extern main
extern Exit
Start:
    call main
    call Exit
    jmp $