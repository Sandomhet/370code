        lw      0       1       one     r1 = 1
        lw      0       2       two     r2 = 2
        add     1       2       3       stall on regB; r1 from WB/END: r3 = 3
        lw      0       4       three   r4 = 3
        add     4       1       5       stall on regA: r5 = 4
        lw      0       4       ptr     r4 = address of slot
        sw      4       5       0       stall on sw base: slot = 4
        lw      0       6       two     r6 = 2
        sw      0       6       slot2   stall on sw data: slot2 = 2
        lw      0       7       ptr     r7 = ptr
        lw      7       7       0       stall on lw base: r7 = slot = 4
        nor     7       7       3       r3 = ~4 = -5
        lw      0       6       one     r6 = 1
        lw      0       6       three   no stall (lw regB is dest): r6 = 3
        lw      0       5       two     r5 = 2
        noop
        add     5       5       5       no stall, lw from MEM/WB: r5 = 4
        lw      0       0       three   lw into r0: no stall, no forward
        add     0       0       2       r2 = 0
        lw      0       1       two     r1 = 2
        beq     1       6       skip    stall on beq, not taken (2 != 3)
        add     1       1       1       r1 = 4
skip    halt
one     .fill   1
two     .fill   2
three   .fill   3
ptr     .fill   slot
slot    .fill   0
slot2   .fill   0
