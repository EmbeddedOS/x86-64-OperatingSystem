CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -I ./libc/include/ -c
all:
	nasm -f elf64 -o syscall.o syscall.asm
	nasm -f elf64 -o start.o start.asm
	gcc $(CFLAGS) stdio.c -o stdio.o
	ar rcs lib.a syscall.o stdio.o
	gcc $(CFLAGS) main.c -o main.o
	ld -nostdlib -T linker.ld -o user start.o main.o lib.a
	objcopy -O binary user user.bin
