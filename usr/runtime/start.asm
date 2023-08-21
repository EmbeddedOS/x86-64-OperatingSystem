section .text
global Start
extern main
extern exit

Start:
    call main
    call exit
    jmp $