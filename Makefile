CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -I ./libc/include/ -c
all:
	nasm -f bin -o boot.bin boot.asm
	nasm -f bin -o loader.bin loader.asm
	nasm -f elf64 -o kernel.o kernel.asm
	nasm -f elf64 -o trapasm.o trap.asm
	gcc $(CFLAGS) main.c -o main.o
	gcc $(CFLAGS) trap.c -o trap.o
	gcc $(CFLAGS) printk.c -o printk.o
	gcc $(CFLAGS) assert.c -o assert.o
	gcc $(CFLAGS) memory.c -o memory.o
	gcc $(CFLAGS) process.c -o process.o
	gcc $(CFLAGS) syscall.c -o syscall.o

	ld -nostdlib -T linker.ld -o kernel kernel.o main.o trap.o trapasm.o printk.o assert.o memory.o process.o syscall.o libc/libc.a
	objcopy -O binary kernel kernel.bin
	dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
	dd if=usr/user.bin of=boot.img bs=512 count=10 seek=106 conv=notrunc

clean:
	rm *.bin *.img
