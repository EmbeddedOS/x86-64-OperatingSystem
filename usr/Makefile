CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -I ../libc/include/ -c
all:
	nasm -f elf64 -o syscall.o syscall.asm
	nasm -f elf64 -o start.o start.asm
	gcc $(CFLAGS) stdio.c -o stdio.o
	ar rcs lib.a syscall.o stdio.o
	gcc $(CFLAGS) process1.c -o process1.o
	gcc $(CFLAGS) process2.c -o process2.o
	gcc $(CFLAGS) process3.c -o process3.o
	gcc $(CFLAGS) shell.c -o shell.o

	ld -nostdlib -T linker.ld -o process1 start.o process1.o lib.a
	objcopy -O binary process1 process1.bin

	ld -nostdlib -T linker.ld -o process2 start.o process2.o lib.a
	objcopy -O binary process2 process2.bin

	ld -nostdlib -T linker.ld -o process3 start.o process3.o lib.a
	objcopy -O binary process3 process3.bin

	ld -nostdlib -T linker.ld -o shell start.o shell.o lib.a ../libc/libc.a
	objcopy -O binary shell shell.bin

clean:
	rm -f *.bin *.img *.o