        lw      0       1       one     r1 = 1
        lw      0       2       base    r2 = address of arr
        noop
        noop
        add     1       1       3       r3 = 2
        sw      2       3       0       data from EX/MEM: arr[0] = 2
        add     3       3       4       r4 = 4
        noop
        sw      2       4       1       data from MEM/WB: arr[1] = 4
        add     4       4       5       r5 = 8
        noop
        noop
        sw      2       5       2       data from WB/END: arr[2] = 8
        add     2       1       6       r6 = base + 1
        sw      6       1       2       base from EX/MEM: arr[3] = 1
        lw      2       7       3       r7 = arr[3] = 1 (sw then lw)
        nor     7       7       7       stall: r7 = -2
        sw      2       7       -1      store below arr (negative offset)
        halt
one     .fill   1
base    .fill   arr
pad     .fill   0
arr     .fill   0
        .fill   0
        .fill   0
        .fill   0
