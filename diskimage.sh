#!/bin/bash
rm boot.img bochsrc

bximage << EOF
1
hd
flat
512
10
boot.img
EOF

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
