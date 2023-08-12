all:
	make -C libc/
	make -C boot/
	make -C kernel/
	make -C usr/

	dd if=boot/boot.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=boot/loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel/kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
	dd if=usr/process1.bin of=boot.img bs=512 count=10 seek=106 conv=notrunc
	dd if=usr/process2.bin of=boot.img bs=512 count=10 seek=116 conv=notrunc
	dd if=usr/shell.bin of=boot.img bs=512 count=10 seek=126 conv=notrunc

image:
	image.sh

clean:
	make -C libc/ clean
	make -C boot/ clean
	make -C kernel/ clean
	make -C usr/ clean
	rm -f *.bin *.img *.o
