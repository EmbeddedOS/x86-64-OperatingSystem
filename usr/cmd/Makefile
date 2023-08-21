CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c
LIBC=../../libc/libc.a ../runtime/runtime.a

INC=-I ../../libc/include/ -I ../runtime/include/
LDFLAGS=-nostdlib -T ../runtime/linker.ld
CPP_LDFLAGS=-nostdlib -T ../runtime/linker.cpp.ld

all:
	gcc $(CFLAGS) $(INC) ls.c -o ls.o
	ld $(LDFLAGS) -o ls.tmp ../runtime/start.o ls.o $(LIBC)
	objcopy -O binary ls.tmp ls.bin

clean:
	rm -f *.bin *.img *.o *.a