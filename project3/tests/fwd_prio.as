        lw      0       1       one     r1 = 1
        noop
        noop
        noop
        add     1       1       2       r2 = 2
        add     2       1       2       r2 = 3 (EX/MEM)
        add     2       1       2       r2 = 4 (EX/MEM)
        add     2       2       3       r2 in EX/MEM, MEM/WB, WB/END: newest wins, r3 = 8
        add     1       1       4       r4 = 2
        add     4       4       4       r4 = 4
        noop
        add     4       0       5       r4 in MEM/WB and WB/END: MEM/WB wins, r5 = 4
        nor     1       1       6       r6 = -2
        add     6       6       6       r6 = -4
        add     1       0       7       r7 = 1 (unrelated)
        add     0       6       7       r6 in MEM/WB and WB/END: r7 = -4
        halt
one     .fill   1
