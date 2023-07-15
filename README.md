# x86-64 Operating System

An Operating System based on x86-64 architecture.

## 1. Make a blank hard disk image

- Before building our OS and make final image, we need a blank hard disk image, We will use `bximage` to make it with:
  - Size 10 megabytes.
  - Sector size is 512 bytes.
  - Kind of image is `flat`.

- this is done by run [image.sh](image.sh) script, the output image is `boot.img` file.

## 2. Simulate the OS

- To simulate the OS, we can use Bochs x86 Emulator 2.7, first generate a bochs configuration file (this is done automatically by run [image.sh](image.sh) script). And then, `make` to build our OS. And finally run `bochs` command to start simulating.

- The second option to simulate our OS is using QEMU, after build our OS with `make`, simply run command: `qemu-system-x86_64 -cpu qemu64,pdpe1gb -hda boot.img`
