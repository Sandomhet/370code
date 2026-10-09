        lw      0       1       base    reg1 = address of d0
        lw      1       2       0       reg2 = mem[d0+0] = 222
        lw      1       3       2       reg3 = mem[d0+2] = 444
        lw      1       4       -1      reg4 = mem[d0-1] = 111, negative offset
        sw      1       3       1       mem[d0+1] = 444, overwrites 333
        lw      1       6       1       reg6 = 444
        sw      0       2       slot    mem[slot] = 222, overwrites 0
        lw      0       7       slot    reg7 = 222
        halt
base    .fill   d0
prev    .fill   111
d0      .fill   222
d1      .fill   333
d2      .fill   444
slot    .fill   0
