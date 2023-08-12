#!/bin/bash
rm boot.img bochsrc

# Create a hard disk image with size 10MB.
bximage << EOF
1
hd
flat
512
100
boot.img
EOF

exit 0

# If use bochs to simulate, we need to make bochsrc file.
bochs << EOF
3
6
26
yes
0
7
1
1
1024
1024


12
3
yes



4
disk
boot.img

20
16
63
512



15
1
disk



4
bochsrc
7
EOF
