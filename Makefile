all:
	make -C libc/
	make -C boot/
	make -C kernel/
	make -C usr/

	dd if=boot/boot.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=boot/loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel/kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
	dd if=usr/shell.bin of=boot.img bs=512 count=20 seek=106 conv=notrunc
	dd if=/dev/zero of=boot.img bs=512 count=$$(expr 204800 - 126) seek=126 conv=notrunc

run:
	make all
	sudo ./mount.sh
	sleep 5
	./qemu.sh

image:
	./image.sh

clean:
	make -C libc/ clean
	make -C boot/ clean
	make -C kernel/ clean
	make -C usr/ clean
	rm -f *.bin *.img *.o
