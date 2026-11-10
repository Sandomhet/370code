        lw      0       1       one     r1 = 1
        lw      0       2       two     r2 = 2
        lw      0       3       five    r3 = 5
        noop
        noop
        noop
        add     1       2       4       r4 = 3
        nor     2       3       5       r5 = ~7 = -8
        add     3       3       6       r6 = 10
        noop
        noop
        sw      0       4       slot    slot = 3
        sw      1       5       slot    slot+1 = -8
        sw      0       6       one     overwrite one = 10
        lw      0       7       slot    r7 = 3
        halt
one     .fill   1
two     .fill   2
five    .fill   5
slot    .fill   0
        .fill   0
