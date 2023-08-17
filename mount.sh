#!/bin/bash

umount /mnt/d/
mount -t vfat boot.img /mnt/d/
echo "Test reading file." > /mnt/d/test.txt