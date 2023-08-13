CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c
LIBC=../libc/libc.a ./runtime/runtime.a
INC=-I ../../libc/include/ -I ./runtime/include/
LDFLAGS=-nostdlib -T linker.ld

all:
	make -C runtime/
	nasm -f elf64 -o start.o start.asm
	gcc $(CFLAGS) $(INC) process1.c -o process1.o
	gcc $(CFLAGS) $(INC) process2.cpp -o process2.o
	gcc $(CFLAGS) $(INC) process3.c -o process3.o
	gcc $(CFLAGS) $(INC) shell.c -o shell.o

	ld $(LDFLAGS) -o process1.tmp start.o process1.o $(LIBC)
	objcopy -O binary process1.tmp process1.bin

	ld $(LDFLAGS) -o process2.tmp start.o process2.o $(LIBC)
	objcopy -O binary process2.tmp process2.bin

	ld $(LDFLAGS) -o process3.tmp start.o process3.o $(LIBC)
	objcopy -O binary process3.tmp process3.bin

	ld $(LDFLAGS) -o shell.tmp start.o shell.o $(LIBC)
	objcopy -O binary shell.tmp shell.bin

clean:
	make -C runtime/ clean
	rm -f *.bin *.img *.o *.a *tmp