CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c
LIBC=../../libc/libc.a
INC=-I ../../libc/include/ -I ./include/

all:
	nasm -f elf64 -o syscall.o syscall.asm
	gcc $(CFLAGS) $(INC) stdio.c -o stdio.o
	ar rcs runtime.a syscall.o stdio.o

clean:
	rm -f *.bin *.img *.o *.a