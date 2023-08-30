#!/bin/bash

umount /mnt/d/
mount -t vfat boot.img /mnt/d/
cp usr/process4.bin /mnt/d/
cp usr/process2.bin /mnt/d/
cp usr/cmd/ls.bin /mnt/d/
cp usr/cmd/clr.bin /mnt/d/

echo "Test reading file." > /mnt/d/test.txt