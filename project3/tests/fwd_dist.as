        lw      0       1       one     r1 = 1
        lw      0       2       two     r2 = 2
        noop
        noop
        noop
        add     1       2       3       r3 = 3
        add     3       1       4       regA from EX/MEM: r4 = 4
        noop
        add     4       2       5       regA from MEM/WB: r5 = 6
        noop
        noop
        add     5       1       6       regA from WB/END: r6 = 7
        noop
        noop
        noop
        add     6       1       7       regA from regfile: r7 = 8
        add     1       7       3       regB from EX/MEM: r3 = 9
        noop
        nor     2       3       4       regB from MEM/WB: r4 = ~11 = -12
        noop
        noop
        add     2       4       5       regB from WB/END: r5 = -10
        add     1       1       6       r6 = 2
        add     2       2       7       r7 = 4
        add     6       7       3       A from MEM/WB, B from EX/MEM: r3 = 6
        nor     7       6       4       A from WB/END, B from EX/MEM... r4 = ~(4|2) = -7
        halt
one     .fill   1
two     .fill   2
