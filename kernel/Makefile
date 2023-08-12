CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c
LDFLAGS=-nostdlib -T linker.ld
LIBC=../libc/libc.a
INC=-I ../libc/include/
all:
	nasm -f elf64 -o kernel.o kernel.asm
	nasm -f elf64 -o trapasm.o trap.asm
	gcc $(CFLAGS) $(INC) main.c -o main.o
	gcc $(CFLAGS) $(INC) trap.c -o trap.o
	gcc $(CFLAGS) $(INC) printk.c -o printk.o
	gcc $(CFLAGS) $(INC) assert.c -o assert.o
	gcc $(CFLAGS) $(INC) memory.c -o memory.o
	gcc $(CFLAGS) $(INC) process.c -o process.o
	gcc $(CFLAGS) $(INC) keyboard.c -o keyboard.o
	gcc $(CFLAGS) $(INC) syscall.c -o syscall.o

	ld $(LDFLAGS) -o kernel 	\
					kernel.o 	\
		 			main.o 		\
					trap.o 		\
					trapasm.o 	\
					printk.o 	\
					assert.o 	\
					memory.o 	\
					process.o 	\
					syscall.o 	\
					keyboard.o  \
					$(LIBC)

	objcopy -O binary kernel kernel.bin

clean:
	rm -f *.bin *.img *.o *.a kernel