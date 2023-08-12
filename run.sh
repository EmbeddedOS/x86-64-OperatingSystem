#!/bin/bash

make && qemu-system-x86_64 -cpu qemu64,pdpe1gb -hda boot.img