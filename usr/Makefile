CFLAGS=-std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c
CPPFLAGS=-mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c

LIBC=../libc/libc.a ./runtime/runtime.a
INC=-I ../../libc/include/ -I ./runtime/include/
LDFLAGS=-nostdlib -T runtime/linker.ld
CPP_LDFLAGS=-nostdlib -T runtime/linker.cpp.ld

all:
	make -C runtime/

	gcc $(CFLAGS) $(INC) process1.c -o process1.o
	g++ $(CPPFLAGS) $(INC) process2.cpp -o process2.o
	gcc $(CFLAGS) $(INC) process3.c -o process3.o
	gcc $(CFLAGS) $(INC) process4.c -o process4.o

	gcc $(CFLAGS) $(INC) shell.c -o shell.o

	ld $(LDFLAGS) -o process1.tmp runtime/start.o process1.o $(LIBC)
	objcopy -O binary process1.tmp process1.bin

	ld $(CPP_LDFLAGS) -o process2.tmp runtime/start.cpp.o process2.o $(LIBC)
	objcopy -O binary process2.tmp process2.bin

	ld $(LDFLAGS) -o process3.tmp runtime/start.o process3.o $(LIBC)
	objcopy -O binary process3.tmp process3.bin

	ld $(LDFLAGS) -o process4.tmp runtime/start.o process4.o $(LIBC)
	objcopy -O binary process4.tmp process4.bin

	ld $(LDFLAGS) -o shell.tmp runtime/start.o shell.o $(LIBC)
	objcopy -O binary shell.tmp shell.bin

clean:
	make -C runtime/ clean
	rm -f *.bin *.img *.o *.a *tmp