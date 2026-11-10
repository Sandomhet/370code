        lw      0       1       one     r1 = 1
        noop
        noop
        noop
        add     1       1       0       writes 2 to r0 (discarded)
        add     0       0       2       must read 0 (no EX/MEM forward): r2 = 0
        add     0       1       3       no MEM/WB forward: r3 = 1
        nor     0       0       0       writes -1 to r0
        noop
        add     1       0       4       no forward: r4 = 1
        add     1       1       0       r0 in EX/MEM
        noop
        beq     0       2       good    r0 = 0 = r2: taken
        add     1       1       5       squashed
        add     1       1       6       squashed
        add     1       1       7       squashed
good    sw      0       0       slot    stores 0
        lw      0       5       slot
        add     5       1       5       r5 = 1
        halt
one     .fill   1
slot    .fill   99
